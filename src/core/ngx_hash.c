
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


void *
ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len)
{
    ngx_uint_t       i;
    ngx_hash_elt_t  *elt;

    elt = hash->buckets[key % hash->size];
    if (elt == NULL) {
        return NULL;
    }

    while (elt->value) {
        if (len != (size_t) elt->len) {
            goto next;
        }

        for (i = 0; i < len; i++) {
            if (name[i] != elt->name[i]) {
                goto next;
            }
        }

        return elt->value;

    next:

        elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
        continue;
    }

    return NULL;
}


void *
ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len)
{
    void        *value;
    ngx_uint_t   i, n, key;

    n = len;

    while (n) {
        if (name[n - 1] == '.') {
            break;
        }
        n--;
    }

    key = 0;

    for (i = n; i < len; i++) {
        key = ngx_hash(key, name[i]);
    }

    value = ngx_hash_find(&hwc->hash, key, &name[n], len - n);
    if (value) {
        /* the 2 low bits of value have the special meaning:
         *     00 - value is data pointer for both "example.com" and "*.example.com";
         *     01 - value is data pointer for "*.example.com" only;
         *     10 - value is pointer to wildcard hash allowing both "example.com" and "*.example.com";
         *     11 - value is pointer to wildcard hash allowing "*.example.com" only.
         */

        if ((uintptr_t) value & 2) {
            if (n == 0) {
                /* "example.com" */
                if ((uintptr_t) value & 1) {
                    return NULL;
                }
                hwc = (ngx_hash_wildcard_t *) ((uintptr_t) value & (uintptr_t) ~3);
                return hwc->value;
            }
            hwc = (ngx_hash_wildcard_t *) ((uintptr_t) value & (uintptr_t) ~3);
            value = ngx_hash_find_wc_head(hwc, name, n - 1);
            if (value) {
                return value;
            }

            return hwc->value;
        }

        if ((uintptr_t) value & 1) {
            if (n == 0) { /* "example.com" */
                return NULL;
            }
            return (void *) ((uintptr_t) value & (uintptr_t) ~3);
        }
        /* very normal */
        return value;
    }
    /* normal hash not found */
    return hwc->value;
}


void *
ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len)
{
    void        *value;
    ngx_uint_t   i, key;

    key = 0;

    for (i = 0; i < len; i++) {
        if (name[i] == '.') {
            break;
        }
        key = ngx_hash(key, name[i]);
    }

    if (i == len) {
        return NULL;
    }

    value = ngx_hash_find(&hwc->hash, key, name, i);

    if (value) {

        /*
         * the 2 low bits of value have the special meaning:
         *     00 - value is data pointer;
         *     11 - value is pointer to wildcard hash allowing "example.*".
         */

        if ((uintptr_t) value & 2) {

            i++;

            hwc = (ngx_hash_wildcard_t *) ((uintptr_t) value & (uintptr_t) ~3);

            value = ngx_hash_find_wc_tail(hwc, &name[i], len - i);

            if (value) {
                return value;
            }

            return hwc->value;
        }

        return value;
    }

    return hwc->value;
}


void *
ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key, u_char *name, size_t len)
{
    void  *value;

    if (hash->hash.buckets) {
        value = ngx_hash_find(&hash->hash, key, name, len);

        if (value) {
            return value;
        }
    }

    if (len == 0) {
        return NULL;
    }

    if (hash->wc_head && hash->wc_head->hash.buckets) {
        value = ngx_hash_find_wc_head(hash->wc_head, name, len);

        if (value) {
            return value;
        }
    }

    if (hash->wc_tail && hash->wc_tail->hash.buckets) {
        value = ngx_hash_find_wc_tail(hash->wc_tail, name, len);

        if (value) {
            return value;
        }
    }

    return NULL;
}

/* 根据 ngx_hash_key_t 算出 ngx_hash_elt_t 的大小(对齐) */
#define NGX_HASH_ELT_SIZE(name) (sizeof(void *) + ngx_align((name)->key.len + 2, sizeof(void *)))

ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
    u_char          *elts;
    size_t           len;
    u_short         *test;
    ngx_uint_t       i, n, key, size, start, bucket_size;
    ngx_hash_elt_t  *elt, **buckets;

    for (n = 0; n < nelts; n++) {
        /* 检查一个bucket能存下任何一个key */
        if (hinit->bucket_size < NGX_HASH_ELT_SIZE(&names[n]) + sizeof(void *)/*单个key结束标志,参见find函数*/) {
            ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0, "could not build the %s, you should " "increase %s_bucket_size: %i", hinit->name, hinit->name, hinit->bucket_size);
            return NGX_ERROR;
        }
    }

    test = ngx_alloc(hinit->max_size * sizeof(u_short), hinit->pool->log);
    if (test == NULL) {
        return NGX_ERROR;
    }

    bucket_size = hinit->bucket_size - sizeof(void *);

    /* start 为最小bucket数量 */
    start = nelts / (bucket_size / (2 * sizeof(void *)));
    start = start ? start : 1;

    if (hinit->max_size > 10000 && nelts && hinit->max_size / nelts < 100) {
        start = hinit->max_size - 1000;
    }

    for (size = start; size <= hinit->max_size; size++) {
        ngx_memzero(test, size * sizeof(u_short));
        for (n = 0; n < nelts; n++) {
            if (names[n].key.data == NULL) {
                continue;
            }
            key = names[n].key_hash % size;
            test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
            if (test[key] > (u_short) bucket_size) {
                goto next;
            }
        }
        goto found;
    next:
        continue;
    }

    ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0, "could not build the %s, you should increase " "either %s_max_size: %i or %s_bucket_size: %i", hinit->name, hinit->name, hinit->max_size, hinit->name, hinit->bucket_size);

    ngx_free(test);

    return NGX_ERROR;

found:
    /* size is final buckets count */
    for (i = 0; i < size; i++) {
        test[i] = sizeof(void *);
    }

    for (n = 0; n < nelts; n++) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
    }

    len = 0;
    /* len is total buckets size */
    for (i = 0; i < size; i++) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        test[i] = (u_short) (ngx_align(test[i], ngx_cacheline_size));
        len += test[i];
    }
    /* 分配hash数组 */
    if (hinit->hash == NULL) {
        hinit->hash = ngx_pcalloc(hinit->pool, sizeof(ngx_hash_wildcard_t) + size * sizeof(ngx_hash_elt_t *));
        if (hinit->hash == NULL) {
            ngx_free(test);
            return NGX_ERROR;
        }
        buckets = (ngx_hash_elt_t **) ((u_char *) hinit->hash + sizeof(ngx_hash_wildcard_t));
    } else {
        buckets = ngx_pcalloc(hinit->pool, size * sizeof(ngx_hash_elt_t *));
        if (buckets == NULL) {
            ngx_free(test);
            return NGX_ERROR;
        }
    }
    /* 分配总的bucket内存 */
    elts = ngx_palloc(hinit->pool, len + ngx_cacheline_size);
    if (elts == NULL) {
        ngx_free(test);
        return NGX_ERROR;
    }

    elts = ngx_align_ptr(elts, ngx_cacheline_size);
    /* 切分内存到各个bucket */
    for (i = 0; i < size; i++) {
        if (test[i] == sizeof(void *)) {
            continue;
        }
        buckets[i] = (ngx_hash_elt_t *) elts;
        elts += test[i];
    }

    for (i = 0; i < size; i++) {
        test[i] = 0;
    }
    /* 插入数据 */
    for (n = 0; n < nelts; n++) {
        if (names[n].key.data == NULL) {
            continue;
        }
        key = names[n].key_hash % size;
        elt = (ngx_hash_elt_t *) ((u_char *) buckets[key] + test[key]);

        elt->value = names[n].value;
        elt->len = (u_short) names[n].key.len;

        ngx_strlow(elt->name, names[n].key.data, names[n].key.len);

        test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
    }

    /* 在每个bucket最后加一个NULL表示一个bucket结束 */
    for (i = 0; i < size; i++) {
        if (buckets[i] == NULL) {
            continue;
        }

        elt = (ngx_hash_elt_t *) ((u_char *) buckets[i] + test[i]);

        elt->value = NULL;
    }

    ngx_free(test);

    hinit->hash->buckets = buckets;
    hinit->hash->size = size;

#if 0

    for (i = 0; i < size; i++) {
        ngx_str_t   val;
        ngx_uint_t  key;

        elt = buckets[i];

        if (elt == NULL) {
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0, "%ui: NULL", i);
            continue;
        }

        while (elt->value) {
            val.len = elt->len;
            val.data = &elt->name[0];
            key = hinit->key(val.data, val.len);
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0, "%ui: %p \"%V\" %ui", i, elt, &val, key);
            elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
        }
    }

#endif

    return NGX_OK;
}


ngx_int_t
ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
    size_t                len, dot_len;
    ngx_uint_t            i, n, dot;
    ngx_array_t           curr_names, next_names;
    ngx_hash_key_t       *name, *next_name;
    ngx_hash_init_t       h;
    ngx_hash_wildcard_t  *wdc;

    if (ngx_array_init(&curr_names, hinit->temp_pool, nelts, sizeof(ngx_hash_key_t)) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_array_init(&next_names, hinit->temp_pool, nelts, sizeof(ngx_hash_key_t)) != NGX_OK) {
        return NGX_ERROR;
    }

    for (n = 0; n < nelts; n = i) { /* names are sorted already */
        dot = 0;
        for (len = 0; len < names[n].key.len; len++) {
            if (names[n].key.data[len] == '.') {
                dot = 1;
                break;
            }
        }
        name = ngx_array_push(&curr_names);
        if (name == NULL) {
            return NGX_ERROR;
        }
        name->key.len = len; /* [site].com */
        name->key.data = names[n].key.data;
        name->key_hash = hinit->key(name->key.data, name->key.len);
        name->value = names[n].value;
        dot_len = len + 1;
        if (dot) {
            len++;
        }
        next_names.nelts = 0;
        if (names[n].key.len != len) {
            next_name = ngx_array_push(&next_names);
            if (next_name == NULL) {
                return NGX_ERROR;
            }
            next_name->key.len = names[n].key.len - len;
            next_name->key.data = names[n].key.data + len;
            next_name->key_hash = 0;
            next_name->value = names[n].value;
        }

        for (i = n + 1; i < nelts; i++) {
            if (ngx_strncmp(names[n].key.data, names[i].key.data, len) != 0) {
                break;
            }

            if (!dot && names[i].key.len > len && names[i].key.data[len] != '.') {
                break;
            }

            next_name = ngx_array_push(&next_names);
            if (next_name == NULL) {
                return NGX_ERROR;
            }

            next_name->key.len = names[i].key.len - dot_len;
            next_name->key.data = names[i].key.data + dot_len;
            next_name->key_hash = 0;
            next_name->value = names[i].value;
        }

        if (next_names.nelts) {
            h = *hinit;
            h.hash = NULL;
            if (ngx_hash_wildcard_init(&h, (ngx_hash_key_t *) next_names.elts, next_names.nelts) != NGX_OK) {
                return NGX_ERROR;
            }
            wdc = (ngx_hash_wildcard_t *) h.hash;
            if (names[n].key.len == len) {
                wdc->value = names[n].value;
            }

            name->value = (void *) ((uintptr_t) wdc | (dot ? 3 : 2));

        } else if (dot) {
            name->value = (void *) ((uintptr_t) name->value | 1);
        }
    }

    if (ngx_hash_init(hinit, (ngx_hash_key_t *) curr_names.elts, curr_names.nelts) != NGX_OK) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


ngx_uint_t
ngx_hash_key(u_char *data, size_t len)
{
    ngx_uint_t  i, key;

    key = 0;

    for (i = 0; i < len; i++) {
        key = ngx_hash(key, data[i]);
    }

    return key;
}


ngx_uint_t
ngx_hash_key_lc(u_char *data, size_t len)
{
    ngx_uint_t  i, key;

    key = 0;

    for (i = 0; i < len; i++) {
        key = ngx_hash(key, ngx_tolower(data[i]));
    }

    return key;
}


ngx_uint_t
ngx_hash_strlow(u_char *dst, u_char *src, size_t n)
{
    ngx_uint_t  key;

    key = 0;

    while (n--) {
        *dst = ngx_tolower(*src);
        key = ngx_hash(key, *dst);
        dst++;
        src++;
    }

    return key;
}


ngx_int_t
ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type)
{
    ngx_uint_t  asize;

    if (type == NGX_HASH_SMALL) {
        asize = 4;
        ha->hsize = 107;

    } else {
        asize = NGX_HASH_LARGE_ASIZE;
        ha->hsize = NGX_HASH_LARGE_HSIZE;
    }

    if (ngx_array_init(&ha->keys, ha->temp_pool, asize, sizeof(ngx_hash_key_t)) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_array_init(&ha->dns_wc_head, ha->temp_pool, asize, sizeof(ngx_hash_key_t)) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_array_init(&ha->dns_wc_tail, ha->temp_pool, asize, sizeof(ngx_hash_key_t)) != NGX_OK) {
        return NGX_ERROR;
    }

    ha->keys_hash = ngx_pcalloc(ha->temp_pool, sizeof(ngx_array_t) * ha->hsize);
    if (ha->keys_hash == NULL) {
        return NGX_ERROR;
    }

    ha->dns_wc_head_hash = ngx_pcalloc(ha->temp_pool, sizeof(ngx_array_t) * ha->hsize);
    if (ha->dns_wc_head_hash == NULL) {
        return NGX_ERROR;
    }

    ha->dns_wc_tail_hash = ngx_pcalloc(ha->temp_pool, sizeof(ngx_array_t) * ha->hsize);
    if (ha->dns_wc_tail_hash == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value, ngx_uint_t flags)
{
    size_t           len;
    u_char          *p;
    ngx_str_t       *name;
    ngx_uint_t       i, k, n, skip, last;
    ngx_array_t     *keys, *hwc;
    ngx_hash_key_t  *hk;

    last = key->len;

    if (flags & NGX_HASH_WILDCARD_KEY) {
        /* supported wildcards: "*.example.com", ".example.com", and "www.example.*" */
        n = 0;
        for (i = 0; i < key->len; i++) {
            if (key->data[i] == '*') {
                if (++n > 1) { /* 不能有2个星 */
                    return NGX_DECLINED;
                }
            }

            if (key->data[i] == '.' && key->data[i + 1] == '.') {
                return NGX_DECLINED;
            }
        }

        if (key->len > 1 && key->data[0] == '.') {
            skip = 1;
            goto wildcard;
        }

        if (key->len > 2) {

            if (key->data[0] == '*' && key->data[1] == '.') {
                skip = 2;
                goto wildcard;
            }

            if (key->data[i - 2] == '.' && key->data[i - 1] == '*') {
                skip = 0;
                last -= 2;
                goto wildcard;
            }
        }

        if (n) { /* 中间有*的情况 */
            return NGX_DECLINED;
        }
    }

    /* exact hash */

    k = 0;

    for (i = 0; i < last; i++) {
        if (!(flags & NGX_HASH_READONLY_KEY)) {
            key->data[i] = ngx_tolower(key->data[i]);
        }
        k = ngx_hash(k, key->data[i]);
    }

    k %= ha->hsize;

    /* check conflicts in exact hash */

    name = ha->keys_hash[k].elts;

    if (name) {
        for (i = 0; i < ha->keys_hash[k].nelts; i++) {
            if (last != name[i].len) {
                continue;
            }

            if (ngx_strncmp(key->data, name[i].data, last) == 0) {
                return NGX_BUSY;
            }
        }

    } else {
        if (ngx_array_init(&ha->keys_hash[k], ha->temp_pool, 4, sizeof(ngx_str_t)) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    name = ngx_array_push(&ha->keys_hash[k]);
    if (name == NULL) {
        return NGX_ERROR;
    }

    *name = *key;

    hk = ngx_array_push(&ha->keys);
    if (hk == NULL) {
        return NGX_ERROR;
    }

    hk->key = *key;
    hk->key_hash = ngx_hash_key(key->data, last);
    hk->value = value;

    return NGX_OK;


wildcard:

    /* wildcard hash */

    k = ngx_hash_strlow(&key->data[skip], &key->data[skip], last - skip);

    k %= ha->hsize;

    if (skip == 1) {

        /* check conflicts in exact hash for ".example.com" */

        name = ha->keys_hash[k].elts;

        if (name) {
            len = last - skip;

            for (i = 0; i < ha->keys_hash[k].nelts; i++) {
                if (len != name[i].len) {
                    continue;
                }

                if (ngx_strncmp(&key->data[1], name[i].data, len) == 0) {
                    return NGX_BUSY;
                }
            }

        } else {
            if (ngx_array_init(&ha->keys_hash[k], ha->temp_pool, 4, sizeof(ngx_str_t)) != NGX_OK) {
                return NGX_ERROR;
            }
        }

        name = ngx_array_push(&ha->keys_hash[k]);
        if (name == NULL) {
            return NGX_ERROR;
        }

        name->len = last - 1;
        name->data = ngx_pnalloc(ha->temp_pool, name->len);
        if (name->data == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(name->data, &key->data[1], name->len);
    }


    if (skip) {

        /*
         * convert "*.example.com" to "com.example.\0"
         *      and ".example.com" to "com.example\0"
         */

        p = ngx_pnalloc(ha->temp_pool, last);
        if (p == NULL) {
            return NGX_ERROR;
        }

        len = 0;
        n = 0;

        for (i = last - 1; i; i--) {
            if (key->data[i] == '.') {
                ngx_memcpy(&p[n], &key->data[i + 1], len);
                n += len;
                p[n++] = '.';
                len = 0;
                continue;
            }

            len++;
        }

        if (len) {
            ngx_memcpy(&p[n], &key->data[1], len);
            n += len;
        }

        p[n] = '\0';

        hwc = &ha->dns_wc_head;
        keys = &ha->dns_wc_head_hash[k];

    } else {

        /* convert "www.example.*" to "www.example\0" */

        last++;

        p = ngx_pnalloc(ha->temp_pool, last);
        if (p == NULL) {
            return NGX_ERROR;
        }

        ngx_cpystrn(p, key->data, last);

        hwc = &ha->dns_wc_tail;
        keys = &ha->dns_wc_tail_hash[k];
    }


    /* check conflicts in wildcard hash */

    name = keys->elts;

    if (name) {
        len = last - skip;

        for (i = 0; i < keys->nelts; i++) {
            if (len != name[i].len) {
                continue;
            }

            if (ngx_strncmp(key->data + skip, name[i].data, len) == 0) {
                return NGX_BUSY;
            }
        }

    } else {
        if (ngx_array_init(keys, ha->temp_pool, 4, sizeof(ngx_str_t)) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    name = ngx_array_push(keys);
    if (name == NULL) {
        return NGX_ERROR;
    }

    name->len = last - skip;
    name->data = ngx_pnalloc(ha->temp_pool, name->len);
    if (name->data == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(name->data, key->data + skip, name->len);


    /* add to wildcard hash */

    hk = ngx_array_push(hwc);
    if (hk == NULL) {
        return NGX_ERROR;
    }

    hk->key.len = last - 1;
    hk->key.data = p;
    hk->key_hash = 0;
    hk->value = value;

    return NGX_OK;
}

u_char *jeff_hash_key_tustring(ngx_hash_key_t *k, u_char*(*e)(void*))
{
   static u_char buffer[1024];
   memset(buffer, 0, sizeof(buffer));
   if (!k) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_hash_key_t[%s/%d=%d:%s]",
                jeff_str_tustring(&k->key), k->key.len, k->key_hash, e ? e(k->value):"");
   return buffer;
}

u_char *jeff_hash_elt_tustring(ngx_hash_elt_t *l, u_char*(*e)(void*))
{
   static u_char buffer[1024];
   memset(buffer, 0, sizeof(buffer));
   if (!l) return "NULL";
   ngx_str_t s = {l->len, l->name};
   ngx_snstrcatf(buffer, sizeof(buffer), "%V", &s);
   ngx_snstrcatf(buffer, sizeof(buffer), ":%s", e ? e(l->value) : "");
   return buffer;
}

u_char *jeff_hash_tustring(ngx_hash_t *h, u_char*(*e)(void*))
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!h) return "NULL";
   ngx_snprintf(buffer, sizeof(buffer), "ngx_hash_t{%d, ", h->size);

   int i;
   for (i = 0; i < h->size; i++) {
      ngx_hash_elt_t *elt = h->buckets[i];
      ngx_snstrcatf(buffer, sizeof(buffer),"%d)", i);
      while (elt->value) {
         ngx_snstrcatf(buffer, sizeof(buffer), "%s ", jeff_hash_elt_tustring(elt, e));
         elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
      }
   }
   ngx_snstrcatf(buffer, sizeof(buffer), "} ");
   return buffer;
}

void __jeff_hash_wildcard_tustring(u_char* buffer, int buflen, ngx_hash_wildcard_t *w, u_char*(*e)(void*))
{
   if (!w) return;
   ngx_snstrcatf(buffer, buflen, "{");
   int i;
   for (i = 0; i < w->hash.size; i++) {
      ngx_hash_elt_t *elt = w->hash.buckets[i];
      ngx_snstrcatf(buffer, buflen, "%d)", i);
      while (elt->value) {
         ngx_str_t s = {elt->len, elt->name};
         ngx_snstrcatf(buffer, buflen, "%V:", &s);
         if ((uintptr_t) elt->value & 2) {
            ngx_hash_wildcard_t * v = (ngx_hash_wildcard_t *) ((uintptr_t) elt->value & (uintptr_t) ~3);
            if (!((uintptr_t) elt->value & 1)) {
               ngx_snstrcatf(buffer, buflen, "%s ", e ? e(v->value) : "");
            }
            __jeff_hash_wildcard_tustring(buffer, buflen, v, e);
         } else {
            void *data = elt->value;
            if ((uintptr_t) elt->value & 1) {
               data = (void *) ((uintptr_t) elt->value & (uintptr_t) ~3);
            }
            ngx_snstrcatf(buffer, buflen, "%s ", e ? e(data) : "");
         }
         elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len, sizeof(void *));
      }
   }
   ngx_snstrcatf(buffer, buflen, "} ");
}

u_char *jeff_hash_wildcard_tustring(ngx_hash_wildcard_t *w, u_char*(*e)(void*))
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!w) return "NULL";
   ngx_snstrcatf(buffer, sizeof(buffer), "ngx_hash_wildcard_t");
   __jeff_hash_wildcard_tustring(buffer, sizeof(buffer), w, e);
   return buffer;
}

u_char *jeff_hash_keys_arrays_tustring(ngx_hash_keys_arrays_t *s, u_char*(*e)(void*))
{
   static u_char buffer[1024 * 8];
   memset(buffer, 0, sizeof(buffer));
   if (!s) return "NULL";
   ngx_snstrcatf(buffer, sizeof(buffer), "ngx_hash_keys_arrays_t{hsize=%d", s->hsize);

   int i, j, c;

   /* keys -- keys_hash */
   if (s->keys.nelts > 0) {
      ngx_snstrcatf(buffer, sizeof(buffer), ", keys=%s", jeff_array_tustring(&s->keys, e));
   }
   for (i = 0; i < s->hsize; i++) {
      ngx_array_t * a = &s->keys_hash[i];
      if (a->nelts > 0) {
         ngx_snstrcatf(buffer, sizeof(buffer), ", keys_hash:%d=", i);
         for (j = 0; j < a->nelts; j++) {
            ngx_snstrcatf(buffer, sizeof(buffer), "%V", &a->elts[j]);
         }
      }
   }

   /* dns_wc_head -- dns_wc_head_hash */
   if (s->dns_wc_head.nelts > 0) {
      ngx_snstrcatf(buffer, sizeof(buffer), ", dns_wc_head=%s", jeff_array_tustring(&s->dns_wc_head, e));
   }
   for (i = 0, c = 0; i < s->hsize; i++) {
      ngx_array_t * a = &s->dns_wc_head_hash[i];
      if (a->nelts > 0) {
         ngx_snstrcatf(buffer, sizeof(buffer), ", dns_wc_head_hash:%d=", i);
         for (j = 0; j < a->nelts; j++) {
            ngx_snstrcatf(buffer, sizeof(buffer), "%V", &a->elts[j]);
         }
      }
   }

   /* dns_wc_tail -- dns_wc_tail_hash */
   if (s->dns_wc_tail.nelts > 0) {
      ngx_snstrcatf(buffer, sizeof(buffer), ", dns_wc_tail=%s", jeff_array_tustring(&s->dns_wc_tail, e));
   }
   for (i = 0, c = 0; i < s->hsize; i++) {
      ngx_array_t * a = &s->dns_wc_tail_hash[i];
      if (a->nelts > 0) {
         ngx_snstrcatf(buffer, sizeof(buffer), ", dns_wc_tail_hash:%d=", i);
         for (j = 0; j < a->nelts; j++) {
            ngx_snstrcatf(buffer, sizeof(buffer), "%V", &a->elts[j]);
         }
      }
   }

   ngx_snstrcatf(buffer, sizeof(buffer), "} ");
   return buffer;
}
