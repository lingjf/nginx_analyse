#include "h2unit.h"

extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
}

ngx_uint_t ut_normal_hash_key(u_char *data, size_t len)
{
   return (ngx_uint_t)data[0];
}

H2UNIT(ngx_hash_normal)
{
   ngx_pool_t * pool;
   void setup()
   {
      ngx_cacheline_size = 16;
      pool = ngx_create_pool(4000, NULL);
   }

   void teardown()
   {
      ngx_destroy_pool(pool);
   }
};


H2CASE(ngx_hash_normal,"init")
{
   ngx_hash_init_t hinit;
   hinit.hash = NULL;
   hinit.max_size = 2;
   hinit.bucket_size = 20;
   hinit.key = ut_normal_hash_key;
   hinit.name = (char*)"hash ut";
   hinit.pool = pool;
   hinit.temp_pool = NULL;
   ngx_hash_key_t names[] = {
      { {2, (u_char*)"a"}, ut_normal_hash_key((u_char*)"a", 2), (void*) "A" },
      { {2, (u_char*)"b"}, ut_normal_hash_key((u_char*)"b", 2), (void*) "B" },
      { {2, (u_char*)"c"}, ut_normal_hash_key((u_char*)"c", 2), (void*) "C" }
   };

   int rv = ngx_hash_init(&hinit, names, sizeof(names) / sizeof(names[0]));
   H2EQ_MATH(NGX_OK, rv);

   H2EQ_STRCMP("ngx_hash_t{2, 0)b:B 1)a:A c:C } ", jeff_hash_tustring(hinit.hash, (u_char*(*)(void*))jeff_tustring));

   void * va = ngx_hash_find(hinit.hash,ut_normal_hash_key((u_char*)"a", 2), (u_char*)"a", 2);
   H2EQ_STRCMP((char*)"A", (char*)va);
   void * vb = ngx_hash_find(hinit.hash,ut_normal_hash_key((u_char*)"b", 2), (u_char*)"b", 2);
   H2EQ_STRCMP((char*)"B", (char*)vb);
   void * vc = ngx_hash_find(hinit.hash,ut_normal_hash_key((u_char*)"c", 2), (u_char*)"c", 2);
   H2EQ_STRCMP((char*)"C", (char*)vc);
}

H2UNIT(ngx_hash_wildcard)
{
   ngx_pool_t * pool;
   void setup()
   {
      ngx_cacheline_size = 16;
      pool = ngx_create_pool(4000, NULL);
   }

   void teardown()
   {
      ngx_destroy_pool(pool);
   }
};

ngx_uint_t ut_wildcard_hash_key(u_char *data, size_t len)
{
   return (ngx_uint_t)data[0];
}

H2CASE(ngx_hash_wildcard,"init 1")
{
   ngx_hash_init_t hinit;
   hinit.hash = NULL;
   hinit.max_size = 4;
   hinit.bucket_size = 30;
   hinit.key = ut_wildcard_hash_key;
   hinit.name = (char*)"hash ut";
   hinit.pool = pool;
   hinit.temp_pool = pool;

   // value should be aligned
   u_char abc_com[8] = "abc.com";
   u_char A[4] = "A";

   ngx_hash_key_t names[] = {
      { {7, abc_com}, ut_wildcard_hash_key(abc_com, 7), (void*) A },
   };
   int rv = ngx_hash_wildcard_init(&hinit, names, sizeof(names) / sizeof(names[0]));
   H2EQ_STRCMP("ngx_hash_wildcard_t{0)abc:{0)com:A } } ",
      jeff_hash_wildcard_tustring((ngx_hash_wildcard_t *)hinit.hash, (u_char*(*)(void*))jeff_tustring));
}

H2CASE(ngx_hash_wildcard,"init 2")
{
   ngx_hash_init_t hinit;
   hinit.hash = NULL;
   hinit.max_size = 4;
   hinit.bucket_size = 30;
   hinit.key = ut_wildcard_hash_key;
   hinit.name = (char*)"hash ut";
   hinit.pool = pool;
   hinit.temp_pool = pool;

   // value should be aligned
   u_char abc_com[8] = "abc.com";
   u_char A[4] = "A";
   u_char abc_dot[8] = "abc.dot";
   u_char B[4] = "B";
   u_char bbc_dot[8] = "bbc.dot";
   u_char C[4] = "C";
   u_char cbc_edu[8] = "cbc.edu";
   u_char D[4] = "D";

   ngx_hash_key_t names[] = {
      { {7, abc_com}, ut_wildcard_hash_key(abc_com, 7), (void*) A },
      { {7, abc_dot}, ut_wildcard_hash_key(abc_dot, 7), (void*) B },
      { {7, bbc_dot}, ut_wildcard_hash_key(bbc_dot, 7), (void*) C },
      { {7, cbc_edu}, ut_wildcard_hash_key(cbc_edu, 7), (void*) D },
   };
   int rv = ngx_hash_wildcard_init(&hinit, names, sizeof(names) / sizeof(names[0]));
   H2EQ_STRCMP("ngx_hash_wildcard_t{0)bbc:{0)dot:C } 1)abc:{0)com:A dot:B } cbc:{0)edu:D } } ",
      jeff_hash_wildcard_tustring((ngx_hash_wildcard_t *)hinit.hash, (u_char*(*)(void*))jeff_tustring));
}

H2CASE(ngx_hash_wildcard,"init 3")
{
   ngx_hash_init_t hinit;
   hinit.hash = NULL;
   hinit.max_size = 4;
   hinit.bucket_size = 30;
   hinit.key = ut_wildcard_hash_key;
   hinit.name = (char*)"hash ut";
   hinit.pool = pool;
   hinit.temp_pool = pool;

   // value should be aligned
   u_char abc_com[8] = "abc.com";
   u_char A[4] = "A";
   u_char abc_com_cn[12] = "abc.com.cn";
   u_char B[4] = "B";

   ngx_hash_key_t names[] = {
      { {7, abc_com}, ut_wildcard_hash_key(abc_com, 7), (void*) A },
      { {10, abc_com_cn}, ut_wildcard_hash_key(abc_com_cn, 10), (void*) B },
   };
   int rv = ngx_hash_wildcard_init(&hinit, names, sizeof(names) / sizeof(names[0]));
   H2EQ_STRCMP("ngx_hash_wildcard_t{0)abc:{0)com:A {0)cn:B } } } ",
      jeff_hash_wildcard_tustring((ngx_hash_wildcard_t *)hinit.hash, (u_char*(*)(void*))jeff_tustring));
}

H2CASE(ngx_hash_wildcard,"find head")
{
   ngx_hash_init_t hinit;
   hinit.hash = NULL;
   hinit.max_size = 4;
   hinit.bucket_size = 30;
   hinit.key = ngx_hash_key;
   hinit.name = (char*)"hash ut";
   hinit.pool = pool;
   hinit.temp_pool = pool;

   // value should be aligned
   u_char abc_com[8] = "com.abc";
   u_char A[4] = "A";
   u_char men_abc_com[12] = "com.abc.men";
   u_char B[4] = "B";
   u_char _xyz_com[12] = "com.xyz.";
   u_char C[4] = "C";
   u_char bbc_dot[8] = "dot.bbc";
   u_char D[4] = "D";
   u_char cnn_edu[8] = "edu.cnn";
   u_char E[4] = "E";

   ngx_hash_key_t names[] = {
      { {7, abc_com}, ngx_hash_key(abc_com, 7), (void*) A },
      { {11, men_abc_com}, ngx_hash_key(men_abc_com, 11), (void*) B },
      { {8, _xyz_com}, ngx_hash_key(_xyz_com, 8), (void*) C },
      { {7, bbc_dot}, ngx_hash_key(bbc_dot, 7), (void*) D },
      { {7, cnn_edu}, ngx_hash_key(cnn_edu, 7), (void*) E },
   };
   int rv = ngx_hash_wildcard_init(&hinit, names, sizeof(names) / sizeof(names[0]));
   H2EQ_MATH(NGX_OK, rv);
   void *data1 = ngx_hash_find_wc_head((ngx_hash_wildcard_t *)hinit.hash, (u_char*)"abc.com", 7);
   H2EQ_STRCMP((char*)A, (char*)data1);
   void *data2 = ngx_hash_find_wc_head((ngx_hash_wildcard_t *)hinit.hash, (u_char*)"men.abc.com", 11);
   H2EQ_STRCMP((char*)B, (char*)data2);
   void *data3 = ngx_hash_find_wc_head((ngx_hash_wildcard_t *)hinit.hash, (u_char*)"xyz.com", 7);
   H2EQ_TRUE(data3 == NULL);
   void *data4 = ngx_hash_find_wc_head((ngx_hash_wildcard_t *)hinit.hash, (u_char*)"boy.xyz.com", 11);
   H2EQ_STRCMP((char*)C, (char*)data4);
   void *data5 = ngx_hash_find_wc_head((ngx_hash_wildcard_t *)hinit.hash, (u_char*)"bbc.dot", 7);
   H2EQ_STRCMP((char*)D, (char*)data5);
   void *data6 = ngx_hash_find_wc_head((ngx_hash_wildcard_t *)hinit.hash, (u_char*)"cat.bbc.dot", 11);
   H2EQ_STRCMP((char*)D, (char*)data6);
}

H2UNIT(ngx_hash_keys_arrays)
{
   ngx_pool_t * pool;
   ngx_hash_keys_arrays_t ka;
   void setup()
   {
      ngx_cacheline_size = 16;
      pool = ngx_create_pool(4000, NULL);


      ka.pool = pool;
      ka.temp_pool = pool;
      int rv = ngx_hash_keys_array_init(&ka, 0);
      H2EQ_MATH(NGX_OK, rv);
   }

   void teardown()
   {
      ngx_destroy_pool(pool);
   }
};

u_char *___test_hash_key_tustring(ngx_hash_key_t *k)
{
   static u_char buffer[1024];
   memset(buffer, 0, sizeof(buffer));
   if (!k) return (u_char*)"NULL";
   ngx_snprintf(buffer, sizeof(buffer), "{%s:%s}",
                jeff_str_tustring(&k->key), jeff_tustring(k->value));
   return buffer;
}

H2CASE(ngx_hash_keys_arrays,"init")
{
   H2EQ_STRCMP("ngx_hash_keys_arrays_t{hsize=10007} ",
                  jeff_hash_keys_arrays_tustring(&ka, (u_char*(*)(void*))___test_hash_key_tustring));
}

H2CASE(ngx_hash_keys_arrays,"add key exact")
{
   char c1[4] = "abc";
   ngx_str_t v1 = {3, (u_char*)c1};
   char d1[4] = "ABC";

   int rv = ngx_hash_add_key(&ka, &v1, d1, 0);
   H2EQ_MATH(NGX_OK, rv);

   H2EQ_STRCMP("ngx_hash_keys_arrays_t{hsize=10007, keys=ngx_array_t{size=16,nelts/alloc=1/16384,{abc:ABC}}, keys_hash:6291=abc} ",
                  jeff_hash_keys_arrays_tustring(&ka, (u_char*(*)(void*))___test_hash_key_tustring));
}

H2CASE(ngx_hash_keys_arrays,"add key wildcard .abc.com")
{
   char c1[12] = ".abc.com";
   ngx_str_t v1 = {8, (u_char*)c1};
   char d1[12] = ".ABC.COM";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_OK, rv);

   H2EQ_STRCMP("ngx_hash_keys_arrays_t{hsize=10007, keys_hash:6159=abc.com, dns_wc_head=ngx_array_t{size=16,nelts/alloc=1/16384,{com.abc:.ABC.COM}}, dns_wc_head_hash:6159=abc.com} ",
                  jeff_hash_keys_arrays_tustring(&ka, (u_char*(*)(void*))___test_hash_key_tustring));
}

H2CASE(ngx_hash_keys_arrays,"add key wildcard *.abc.com")
{
   char c1[12] = "*.abc.com";
   ngx_str_t v1 = {9, (u_char*)c1};
   char d1[12] = "*.ABC.COM";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_OK, rv);

   H2EQ_STRCMP("ngx_hash_keys_arrays_t{hsize=10007, dns_wc_head=ngx_array_t{size=16,nelts/alloc=1/16384,{com.abc.:*.ABC.COM}}, dns_wc_head_hash:6159=abc.com} ",
                  jeff_hash_keys_arrays_tustring(&ka, (u_char*(*)(void*))___test_hash_key_tustring));
}

H2CASE(ngx_hash_keys_arrays,"add key wildcard abc.com consider as exact hash")
{
   char c1[8] = "abc.com";
   ngx_str_t v1 = {7, (u_char*)c1};
   char d1[8] = "ABC.COM";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_OK, rv);

   H2EQ_STRCMP("ngx_hash_keys_arrays_t{hsize=10007, keys=ngx_array_t{size=16,nelts/alloc=1/16384,{abc.com:ABC.COM}}, keys_hash:6159=abc.com} ",
                  jeff_hash_keys_arrays_tustring(&ka, (u_char*(*)(void*))___test_hash_key_tustring));
}

H2CASE(ngx_hash_keys_arrays,"add key wildcard www.abc.*")
{
   char c1[12] = "www.abc.*";
   ngx_str_t v1 = {9, (u_char*)c1};
   char d1[12] = "WWW.ABC.*";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_OK, rv);

   H2EQ_STRCMP("ngx_hash_keys_arrays_t{hsize=10007, dns_wc_tail=ngx_array_t{size=16,nelts/alloc=1/16384,{www.abc:WWW.ABC.*}}, dns_wc_tail_hash:1315=www.abc.} ",
                  jeff_hash_keys_arrays_tustring(&ka, (u_char*(*)(void*))___test_hash_key_tustring));
}

H2CASE(ngx_hash_keys_arrays,"error case : add key wildcard www.*.com")
{
   char c1[12] = "www.*.com";
   ngx_str_t v1 = {9, (u_char*)c1};
   char d1[12] = "WWW.*.COM";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_DECLINED, rv);
}

H2CASE(ngx_hash_keys_arrays,"error case : add key wildcard www.*.*")
{
   char c1[8] = "www.*.*";
   ngx_str_t v1 = {7, (u_char*)c1};
   char d1[8] = "WWW.*.*";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_DECLINED, rv);
}

H2CASE(ngx_hash_keys_arrays,"error case : add key wildcard *..com")
{
   char c1[8] = "*..com";
   ngx_str_t v1 = {6, (u_char*)c1};
   char d1[8] = "*..COM";

   int rv = ngx_hash_add_key(&ka, &v1, d1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_DECLINED, rv);
}

H2UNIT(ngx_hash_combined)
{
   ngx_pool_t * pool;
   ngx_hash_keys_arrays_t ka;
   void setup()
   {
      ngx_cacheline_size = 16;
      pool = ngx_create_pool(4000, NULL);


      ka.pool = pool;
      ka.temp_pool = pool;
      int rv = ngx_hash_keys_array_init(&ka, 0);
      H2EQ_MATH(NGX_OK, rv);
   }

   void teardown()
   {
      ngx_destroy_pool(pool);
   }
};

H2CASE(ngx_hash_combined,"hi")
{
   int rv;

   char c1[] = "*.abc.com";
   ngx_str_t k1 = {strlen(c1), (u_char*)c1};
   char v1[] = "A";

   rv = ngx_hash_add_key(&ka, &k1, v1, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_OK, rv);

   char c2[] = "*.bbc.com";
   ngx_str_t k2 = {strlen(c2), (u_char*)c2};
   char v2[] = "B";

   rv = ngx_hash_add_key(&ka, &k2, v2, NGX_HASH_WILDCARD_KEY);
   H2EQ_MATH(NGX_OK, rv);

   ngx_hash_init_t             hash;

   hash.key = ngx_hash_key_lc;
   hash.max_size = 4;
   hash.bucket_size = 40;
   hash.name = "hello";
   hash.pool = pool;
   hash.temp_pool = pool;

   hash.hash = NULL;

   rv = ngx_hash_wildcard_init(&hash, (ngx_hash_key_t*)ka.dns_wc_head.elts, ka.dns_wc_head.nelts);
   H2EQ_MATH(NGX_OK, rv);
   ngx_hash_wildcard_t *wc_head = (ngx_hash_wildcard_t *) hash.hash;
   H2EQ_TRUE(wc_head);

   //H2EQ_STRCMP("ngx_hash_wildcard_t{0)com:{0)abc:A } } } ",
   //      jeff_hash_wildcard_tustring(wc_head, (u_char*(*)(void*))jeff_tustring));

   char n1[] = "www.abc.com";
   void * r1 = ngx_hash_find_wc_head(wc_head, (u_char*)n1, strlen(n1));

   H2EQ_STRCMP((char*)"A", (char*)r1);


   //char n2[] = "www.bbc.com";
   //void * r2 = ngx_hash_find_wc_head(wc_head, (u_char*)n2, strlen(n2));

   //H2EQ_STRCMP((char*)"B", (char*)r2);

}


