/* <DESC>
 * Source code using the multi interface to download many
 * files, with a capped maximum amount of simultaneous transfers.
 * </DESC>
 * Written by Michael Wallner
 */ 
 
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/multi.h>
 
static const char *urls[] = {
"https://192.168.0.1/pages/go.com/go.com/index.html",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/css/css_pbm0lsQQJ7A7WCCIMgxLho6mI_kBNgznNUWmTWcnfoE.css",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/css/css_I1czyKE8omE0sKMWyEvG09Hupq8ESIfxXYNCx6vMDA8.css",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/css/css_Wu8npAzy16WmnnnWKxpexfgsAryolGGaX6yO3GWA5bU.css",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/css/css_i1JVoLuNTcH9cN2eywYdFavHZ65fEur10nYzZpdtQp4.css",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/css/css_HhTpF_NoAQO-RKrDUOq-skhVmSzaCsswLBMrtqb14v0.css",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/js/js_xAPl0qIk9eowy_iS9tNkCWXLUVoat94SQT48UBCFkyQ.js",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/js/js_enH0d4Aw5YKYJp6q8loPDxMdJyqAMdVKVW1bMcX4itY.js",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/js/js_go466f5pxBTJOD6brXg42PZjRdNUWcAlHzNobReC6cs.js",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/js/js_43n5FBy8pZxQHxPXkf-sQF7ZiacVZke14b0VlvSA554.js",
"https://192.168.0.1/pages/go.com/ajax.googleapis.com/ajax/libs/jquery/1.6.3/jquery.min.js",
"https://192.168.0.1/pages/go.com/aglobal.go.com/stat/dolWebAnalytics.js",
"https://192.168.0.1/pages/bbc.co.uk/www.googletagservices.com/tag/js/gpt.js",
"https://192.168.0.1/pages/go.com/www.googletagservices.com/tag/js/gpt_mobile.js",
"https://192.168.0.1/pages/go.com/vp.disney.go.com/js.add.js",
"https://192.168.0.1/pages/go.com/pagead2.googlesyndication.com/pagead/show_companion_ad.js",
"https://192.168.0.1/pages/imgur.com/partner.googleadservices.com/gpt/pubads_impl_25.js",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/header.jpg",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/ad-slug-vertical.gif",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/logo.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/disney_online.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/di_entertainment.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/di_family.png",
"https://192.168.0.1/missing/pubads.g.doubleclick.net/gampad/ads",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/abc_news.png",
"https://192.168.0.1/missing/pagead2.googlesyndication.com/pagead/imgad",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/di_games.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/espn.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/abc.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/disney_park.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/files/disney_shopping.png",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/footer_logo.png",
"https://192.168.0.1/pages/go.com/vp.disney.go.com/player/latest/html5/plugins/css/player.css",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/pixie_dust.jpg",
"https://192.168.0.1/pages/youtube.com/pagead2.googlesyndication.com/pagead/osd.js",
"https://192.168.0.1/missing/pubads.g.doubleclick.net/gampad/ads",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/ad-slug-horizontal.gif",
"https://192.168.0.1/missing/globalregsession.go.com/globalregsession/session",
"https://192.168.0.1/missing/ad.doubleclick.net/N7046/adj/goc/houseads_sz_160x604_click_https_",
"https://192.168.0.1/missing/ohlone.vizu.com/a.gif",
"https://192.168.0.1/missing/pagead2.googlesyndication.com/simgad/2665363266969817265",
"https://192.168.0.1/pages/linkedin.com/www.google-analytics.com/ga.js",
"https://192.168.0.1/missing/speed.pointroll.com/PointRoll/Media/Banners/tran1x1.gif",
"https://192.168.0.1/pages/go.com/go.com/sites/default/themes/go_com/images/footer.png",
"https://192.168.0.1/pages/go.com/aglobal.go.com/stat/s_code.js",
"https://192.168.0.1/pages/go.com/vp.disney.go.com/player/latest/flash/kdp3.swf",
"https://192.168.0.1/missing/www.google-analytics.com/__utm.gif",
"https://192.168.0.1/pages/go.com/cdn.adimages.go.com/ad/sponsors/utilities/disney/dol-audience-science_v1.html/index.html",
"https://192.168.0.1/pages/go.com/secure-us.imrworldwide.com/novms/js/2/ggcmb390.js",
"https://192.168.0.1/missing/secure-us.imrworldwide.com/cgi-bin/m",
"https://192.168.0.1/missing/ctologger01.analytics.go.com/cto",
"https://192.168.0.1/missing/weblogger01.data.disney.com",
"https://192.168.0.1/pages/go.com/js.revsci.net/gateway/gw.js",
"https://192.168.0.1/missing/pix04.revsci.net/A08723/b3/0/3/120814/73689855.js",
"https://192.168.0.1/missing/b.scorecardresearch.com/r",
};
 
#define MAX 6 
#define CNT sizeof(urls)/sizeof(char*)  
 
static size_t cb(char *d, size_t n, size_t l, void *p)
{
  /* take care of the data here, ignored in this example */ 
  (void)d;
  (void)p;
  return n*l;
}
 
static void init(CURLM *cm, int i)
{
  CURL *eh = curl_easy_init();
 
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, cb);
   curl_easy_setopt(eh, CURLOPT_TCP_NODELAY, 1L);
  curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
  curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
  curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
  curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(eh, CURLOPT_SSL_VERIFYHOST, 0L);
 
#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */ 
    curl_easy_setopt(eh, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
 
    /* Perform the request, res will get the return code */ 
 
  curl_multi_add_handle(cm, eh);
}
 
int main(void)
{
  CURLM *cm;
  CURLMsg *msg;
  long L;
  unsigned int C=0;
  int M, Q, U = -1;
  fd_set R, W, E;
  struct timeval T;
  int res;
  double bytes=0;
  long header_bytes=0;
  double transfer_time=0;
  unsigned long page_size=0;
  
  struct timeval start, stop;
  gettimeofday(&start, NULL);
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  cm = curl_multi_init();
 
  /* we can optionally limit the total amount of connections this multi handle
     uses */ 
  curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX);
 
  for(C = 0; C < MAX; ++C) {
    init(cm, C);
  }
 
  while(U) {
    curl_multi_perform(cm, &U);
 
    if(U) {
      FD_ZERO(&R);
      FD_ZERO(&W);
      FD_ZERO(&E);
 
      if(curl_multi_fdset(cm, &R, &W, &E, &M)) {
        fprintf(stderr, "E: curl_multi_fdset\n");
        return EXIT_FAILURE;
      }
 
      if(curl_multi_timeout(cm, &L)) {
        fprintf(stderr, "E: curl_multi_timeout\n");
        return EXIT_FAILURE;
      }
      if(L == -1)
        L = 100;
 
      if(M == -1) {
#ifdef WIN32
        Sleep(L);
#else
        sleep((unsigned int)L / 1000);
#endif
      }
      else {
        T.tv_sec = L/1000;
        T.tv_usec = (L%1000)*1000;
 
        if(0 > select(M+1, &R, &W, &E, &T)) {
          fprintf(stderr, "E: select(%i,,,,%li): %i: %s\n",
              M+1, L, errno, strerror(errno));
          return EXIT_FAILURE;
        }
      }
    }
 
    while((msg = curl_multi_info_read(cm, &Q))) {
      if(msg->msg == CURLMSG_DONE) {
        char *url;
        CURL *e = msg->easy_handle;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
        fprintf(stderr, "R: %d - %s <%s>\n",
                msg->data.result, curl_easy_strerror(msg->data.result), url);
        if((res = curl_easy_getinfo(msg->easy_handle, CURLINFO_SIZE_DOWNLOAD, &bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(msg->easy_handle, CURLINFO_HEADER_SIZE, &header_bytes)) != CURLE_OK ||
				(res = curl_easy_getinfo(msg->easy_handle, CURLINFO_TOTAL_TIME, &transfer_time)) != CURLE_OK ) {
				fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
			}
		gettimeofday(&stop, NULL);	
		page_size+=(long)bytes+header_bytes;
		double time=((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000);	
		printf("[%f]Time: %f Size: %lu\n", time,transfer_time, (long)bytes+header_bytes);
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
      }
      else {
        fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
      }
      if(C < CNT) {
        init(cm, C++);
        U++; /* just to prevent it from remaining at 0 if there are more
                URLs to get */ 
      }
    }
  }
 
  curl_multi_cleanup(cm);
  curl_global_cleanup();
  printf("Page size: %ld\n",page_size);
 
  return EXIT_SUCCESS;
}

