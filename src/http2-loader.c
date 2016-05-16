/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * Multiplexed HTTP/2 downloads over a single connection
 * </DESC>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* somewhat unix-specific */
#include <sys/time.h>
#include <unistd.h>

/* curl stuff */
#include <curl/curl.h>

static const char *urls[] = {
"https://192.168.0.1:8000/pages/go.com/go.com/index.html",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/css/css_pbm0lsQQJ7A7WCCIMgxLho6mI_kBNgznNUWmTWcnfoE.css",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/css/css_I1czyKE8omE0sKMWyEvG09Hupq8ESIfxXYNCx6vMDA8.css",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/css/css_Wu8npAzy16WmnnnWKxpexfgsAryolGGaX6yO3GWA5bU.css",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/css/css_i1JVoLuNTcH9cN2eywYdFavHZ65fEur10nYzZpdtQp4.css",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/css/css_HhTpF_NoAQO-RKrDUOq-skhVmSzaCsswLBMrtqb14v0.css",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/js/js_xAPl0qIk9eowy_iS9tNkCWXLUVoat94SQT48UBCFkyQ.js",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/js/js_enH0d4Aw5YKYJp6q8loPDxMdJyqAMdVKVW1bMcX4itY.js",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/js/js_go466f5pxBTJOD6brXg42PZjRdNUWcAlHzNobReC6cs.js",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/js/js_43n5FBy8pZxQHxPXkf-sQF7ZiacVZke14b0VlvSA554.js",
"https://192.168.0.1:8000/pages/go.com/ajax.googleapis.com/ajax/libs/jquery/1.6.3/jquery.min.js",
"https://192.168.0.1:8000/pages/go.com/aglobal.go.com/stat/dolWebAnalytics.js",
"https://192.168.0.1:8000/pages/bbc.co.uk/www.googletagservices.com/tag/js/gpt.js",
"https://192.168.0.1:8000/pages/go.com/www.googletagservices.com/tag/js/gpt_mobile.js",
"https://192.168.0.1:8000/pages/go.com/vp.disney.go.com/js.add.js",
"https://192.168.0.1:8000/pages/go.com/pagead2.googlesyndication.com/pagead/show_companion_ad.js",
"https://192.168.0.1:8000/pages/imgur.com/partner.googleadservices.com/gpt/pubads_impl_25.js",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/header.jpg",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/ad-slug-vertical.gif",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/logo.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/disney_online.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/di_entertainment.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/di_family.png",
"https://192.168.0.1:8000/missing/pubads.g.doubleclick.net/gampad/ads",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/abc_news.png",
"https://192.168.0.1:8000/missing/pagead2.googlesyndication.com/pagead/imgad",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/di_games.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/espn.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/abc.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/disney_park.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/files/disney_shopping.png",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/footer_logo.png",
"https://192.168.0.1:8000/pages/go.com/vp.disney.go.com/player/latest/html5/plugins/css/player.css",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/pixie_dust.jpg",
"https://192.168.0.1:8000/pages/youtube.com/pagead2.googlesyndication.com/pagead/osd.js",
"https://192.168.0.1:8000/missing/pubads.g.doubleclick.net/gampad/ads",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/ad-slug-horizontal.gif",
"https://192.168.0.1:8000/missing/globalregsession.go.com/globalregsession/session",
"https://192.168.0.1:8000/missing/ad.doubleclick.net/N7046/adj/goc/houseads_sz_160x604_click_http_",
"https://192.168.0.1:8000/missing/ohlone.vizu.com/a.gif",
"https://192.168.0.1:8000/missing/pagead2.googlesyndication.com/simgad/2665363266969817265",
"https://192.168.0.1:8000/pages/linkedin.com/www.google-analytics.com/ga.js",
"https://192.168.0.1:8000/missing/speed.pointroll.com/PointRoll/Media/Banners/tran1x1.gif",
"https://192.168.0.1:8000/pages/go.com/go.com/sites/default/themes/go_com/images/footer.png",
"https://192.168.0.1:8000/pages/go.com/aglobal.go.com/stat/s_code.js",
"https://192.168.0.1:8000/pages/go.com/vp.disney.go.com/player/latest/flash/kdp3.swf",
"https://192.168.0.1:8000/missing/www.google-analytics.com/__utm.gif",
"https://192.168.0.1:8000/pages/go.com/cdn.adimages.go.com/ad/sponsors/utilities/disney/dol-audience-science_v1.html/index.html",
"https://192.168.0.1:8000/pages/go.com/secure-us.imrworldwide.com/novms/js/2/ggcmb390.js",
"https://192.168.0.1:8000/missing/secure-us.imrworldwide.com/cgi-bin/m",
"https://192.168.0.1:8000/missing/ctologger01.analytics.go.com/cto",
"https://192.168.0.1:8000/missing/weblogger01.data.disney.com",
"https://192.168.0.1:8000/pages/go.com/js.revsci.net/gateway/gw.js",
"https://192.168.0.1:8000/missing/pix04.revsci.net/A08723/b3/0/3/120814/73689855.js",
"https://192.168.0.1:8000/missing/b.scorecardresearch.com/r",

};

#define CNT sizeof(urls)/sizeof(char*) 

#ifndef CURLPIPE_MULTIPLEX
/* This little trick will just make sure that we don't enable pipelining for
   libcurls old enough to not have this symbol. It is _not_ defined to zero in
   a recent libcurl header. */
#define CURLPIPE_MULTIPLEX 0
#endif

#define NUM_HANDLES 1000

void *curl_hnd[NUM_HANDLES];
int num_transfers;

/* a handle to number lookup, highly ineffective when we do many
   transfers... */
static int hnd2num(CURL *hnd)
{
  int i;
  for(i=0; i< num_transfers; i++) {
    if(curl_hnd[i] == hnd)
      return i;
  }
  return 0; /* weird, but just a fail-safe */
}

static
void dump(const char *text, int num, unsigned char *ptr, size_t size,
          char nohex)
{
  size_t i;
  size_t c;

  unsigned int width=0x10;

  if(nohex)
    /* without the hex output, we can fit more on screen */
    width = 0x40;

  fprintf(stderr, "%d %s, %ld bytes (0x%lx)\n",
          num, text, (long)size, (long)size);

  for(i=0; i<size; i+= width) {

    fprintf(stderr, "%4.4lx: ", (long)i);

    if(!nohex) {
      /* hex not disabled, show it */
      for(c = 0; c < width; c++)
        if(i+c < size)
          fprintf(stderr, "%02x ", ptr[i+c]);
        else
          fputs("   ", stderr);
    }

    for(c = 0; (c < width) && (i+c < size); c++) {
      /* check for 0D0A; if found, skip past and start a new line of output */
      if(nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
        i+=(c+2-width);
        break;
      }
      fprintf(stderr, "%c",
              (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */
      if(nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
        i+=(c+3-width);
        break;
      }
    }
    fputc('\n', stderr); /* newline */
  }
}

static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  const char *text;
  int num = hnd2num(handle);
  (void)handle; /* prevent compiler warning */
  (void)userp;
  switch (type) {
  case CURLINFO_TEXT:
    fprintf(stderr, "== %d Info: %s", num, data);
  default: /* in case a new one is introduced to shock us */
    return 0;

  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }

  dump(text, num, (unsigned char *)data, size, 1);
  return 0;
}

static size_t cb(char *d, size_t n, size_t l, void *p)
{
  /* take care of the data here, ignored in this example */ 
  (void)d;
  (void)p;
  return n*l;
}

static void setup(CURL *hnd, int num)
{


  /* write to this file */
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, cb);

  /* set the same URL */
  curl_easy_setopt(hnd, CURLOPT_URL,urls[num]);
  
  curl_easy_setopt(hnd, CURLOPT_TCP_NODELAY, 1L);

  /* send it verbose for max debuggaility */
  //curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(hnd, CURLOPT_DEBUGFUNCTION, my_trace);

  /* HTTP/2 please */
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

  /* we use a self-signed test server, skip verification during debugging */
  curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

#if (CURLPIPE_MULTIPLEX > 0)
  /* wait for pipe connection to confirm */
  curl_easy_setopt(hnd, CURLOPT_PIPEWAIT, 1L);
#endif

  curl_hnd[num] = hnd;
}

/*
 * Simply download two files over HTTP/2, using the same physical connection!
 */
int main(int argc, char **argv)
{
  CURL *easy[NUM_HANDLES];
  CURLM *multi_handle;
  int i;
  int still_running; /* keep number of running handles */
  struct timeval start, stop;
  gettimeofday(&start, NULL);

 
  /* init a multi stack */
  multi_handle = curl_multi_init();

  for(i=0; i<CNT; i++) {
    easy[i] = curl_easy_init();
    /* set options */
    setup(easy[i], i);

    /* add the individual transfer */
    curl_multi_add_handle(multi_handle, easy[i]);
  }

  curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

  /* we start some action by calling perform right away */
  curl_multi_perform(multi_handle, &still_running);

  do {
    struct timeval timeout;
    int rc; /* select() return code */
    CURLMcode mc; /* curl_multi_fdset() return code */

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;

    long curl_timeo = -1;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    curl_multi_timeout(multi_handle, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    if(mc != CURLM_OK) {
      fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
      break;
    }

    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */

    if(maxfd == -1) {
#ifdef _WIN32
      Sleep(100);
      rc = 0;
#else
      /* Portable sleep for platforms other than Windows. */
      struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
      rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else {
      /* Note that on some platforms 'timeout' may be modified by select().
         If you need access to the original value save a copy beforehand. */
      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
    }

    switch(rc) {
    case -1:
      /* select error */
      break;
    case 0:
    default:
      /* timeout or readable/writable sockets */
      curl_multi_perform(multi_handle, &still_running);
      break;
    }
  } while(still_running);
  
     gettimeofday(&stop, NULL);
  
    double bytes,avj_obj_size;
    long total_bytes;
    long header_bytes;
    double transfer_time;
    unsigned long  total_transfer_time;
    int res;
    long *obj_sz=(long*)malloc(CNT*sizeof(long));
    if(obj_sz==NULL){
	printf("Memory allocation error!\n");
	exit(0);
    }

    double * obj_load_time=(double*)malloc(CNT*sizeof(double));
    if(obj_load_time==NULL){ 
	printf("Memory allocation error!\n");
	exit(0);
    }
  
  /* check for download performance */ 
	for(i=0; i<CNT; i++) {
		if((res = curl_easy_getinfo(easy[i], CURLINFO_SIZE_DOWNLOAD, &bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(easy[i], CURLINFO_HEADER_SIZE, &header_bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(easy[i], CURLINFO_TOTAL_TIME, &obj_load_time[i])) != CURLE_OK ) {
				fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
			}
     obj_sz[i]=(long)bytes+header_bytes;
     total_bytes+=obj_sz[i];
    }
  
	printf("\"object_size\": [");	 
	for(i=0; i<CNT; i++)
			if (i!=(CNT-1)) 
				printf("%lu,",obj_sz[i]);
			else
				printf("%lu],",obj_sz[i]);
  
	printf("\"object_load_time\": [ ");	 
	for(i=0; i<CNT; i++) 
			if (i!=(CNT-1)) 
				printf("%f,",obj_load_time[i]);
			else
				printf("%f],",obj_load_time[i]);
		

	total_transfer_time=((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000);
	printf("\"page_load_time\": %f,", (double)total_transfer_time/1000);
 
	printf("\"page_size\": %lu,\"average_object_size\": %f }\n", total_bytes, (double)total_bytes/CNT);

  curl_multi_cleanup(multi_handle);

  for(i=0; i<CNT; i++)
    curl_easy_cleanup(easy[i]);

  return 0;
}
