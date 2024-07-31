#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <linux/fb.h>
#include <linux/kernel.h>
#include <ion_mem_alloc.h>
#include <videoOutPort.h>


#if 1
#define LOGD(fmt,args...) do {}while(0)
//#define LOGD(fmt,args...) do {printf(fmt,##args);}while(0)
#define LOGI(fmt,args...) do {printf(fmt,##args);}while(0)
#else
#define LOGD(fmt,args...) do {}while(0)
#define LOGI(fmt,args...) do {}while(0)
#endif
#define LOGW(fmt,args...) do {printf(fmt,##args);}while(0)
#define LOGE(fmt,args...) do {printf(fmt,##args);}while(0)



char* pPhyBuf;
char* pVirBuf;
unsigned int fbAddr[3];
videoParam vparam;
renderBuf rBuf;

dispOutPort *mDispOutPort=NULL;
struct SunxiMemOpsS *pMemops;

struct test_layer_info
{
	int fb_width, fb_height;
	int width,height;//screen size
	int fh;//picture resource file handle
	int format;
	int buffer_num;//is double buffer
	char filename[64];
	char formatname[32];
	int loop;
	int flg_terminal;
};
struct test_layer_info test_info;
/* Signal handler */

extern int udisp_running;

static void terminate(int sig_no)
{
	int try=0;
	LOGI("Got signal %d, exiting ...\n", sig_no);
	
	test_info.flg_terminal=1;

	while(test_info.flg_terminal && try++ <10){
		usleep(300000);
		LOGI("%s %d try:%d %d %d\n", __func__,__LINE__,try,test_info.flg_terminal,udisp_running);
	}
	udisp_running=0;
	usleep(300000);
	LOGI("%s %d try:%d %d %d\n", __func__,__LINE__,try,test_info.flg_terminal,udisp_running);
	if (mDispOutPort)
		mDispOutPort->setEnable(mDispOutPort, 0);
	LOGI("%s %d try:%d\n", __func__,__LINE__,try);

	if (pMemops && pVirBuf) {
		SunxiMemPfree(pMemops, pVirBuf);
		SunxiMemClose(pMemops);
	}
	LOGI("%s %d try:%d\n", __func__,__LINE__,try);

	if (test_info.fh !=-1)
		close(test_info.fh);
	LOGI("%s %d try:%d\n", __func__,__LINE__,try);

	if (mDispOutPort) {
		mDispOutPort->deinit(mDispOutPort);
		DestroyVideoOutport(mDispOutPort);
	}

	LOGI("exit\n");
	exit(1);
}

int parse_cmdline(struct test_layer_info *p)
{
	p->format = VIDEO_PIXEL_FORMAT_NV21;
	sprintf(p->filename,"%s","/proc/udisp/xfz1986");
	//sprintf(p->filename,"%s","/sx.jpg");
	LOGD("filename=%s\n", p->filename);
	p->fb_width = 480;
	p->fb_height = 480;
	p->loop =9999;
	p->buffer_num = 1;
	LOGD("loop=%d\n",p->loop);
	return 0;
}

static void install_sig_handler(void)
{
	signal(SIGBUS, terminate);
	signal(SIGFPE, terminate);
	signal(SIGHUP, terminate);
	signal(SIGILL, terminate);
	signal(SIGINT, terminate);
	signal(SIGIOT, terminate);
	signal(SIGPIPE, terminate);
	signal(SIGQUIT, terminate);
	signal(SIGSEGV, terminate);
	signal(SIGSYS, terminate);
	signal(SIGTERM, terminate);
	signal(SIGTRAP, terminate);
	signal(SIGUSR1, terminate);
	signal(SIGUSR2, terminate);
}

static void addrSetting();


#include <jpegdecode.h>
#include <errno.h>


static char * readSrcData(char *path, int *pLen)
{
    FILE *fp = NULL;
    int ret = 0;
    char *data = NULL;

    fp = fopen(path, "rb");
    if(fp == NULL)
    {
    	*pLen =0;
        LOGW("read jpeg file error, errno(%d)\n", errno);
        return NULL;
    }
    *pLen= 512*1000;
    data = (char *) malloc (sizeof(char)*(*pLen));

    if(data == NULL)
      {
          LOGD("malloc memory fail\n");
          fclose(fp);
          return NULL;
      }

    ret = fread (data,1,*pLen,fp);
    if (ret != *pLen)
    {
        LOGD("read src file fail %d %d \n",ret,*pLen);
		*pLen =ret;
    }

    if(fp != NULL)
    {
        fclose(fp);
    }
    return data;
}




typedef struct{
	JpegDecoder* jpegdecoder;
	char* srcBuf;
	int srcBufLen;
	JpegDecodeScaleDownRatio scaleRatio;
	JpegDecodeOutputDataType outputDataTpe;
	ImgFrame* imgFrame ;


} deocde_jpg_mgr_t;

int decode_jpg_create(deocde_jpg_mgr_t  * mgr){
	static int flg=1;

	if(mgr->jpegdecoder == NULL)
	{
		LOGI("create mgr\n");
		mgr->jpegdecoder = JpegDecoderCreate(flg);
		 
		flg=0;
	    if(NULL == mgr->jpegdecoder)
	    {
	        LOGE("create jpegdecoder failed\n");
	        return -1;
	    }
		
		mgr->scaleRatio = JPEG_DECODE_SCALE_DOWN_1;
	    mgr->outputDataTpe= JpegDecodeOutputDataNV21;
	}

}

int decode_jpg_prepare_data(deocde_jpg_mgr_t * mgr,char  * fname){


    mgr->srcBuf = readSrcData(fname,&mgr->srcBufLen);

}

int decode_jpg_destroy(deocde_jpg_mgr_t * mgr)
{
	LOGI("destroy mgr\n");
    JpegDecoderDestory(mgr->jpegdecoder);
	mgr->jpegdecoder =NULL;

}

int decode_jpg(deocde_jpg_mgr_t * mgr)
{

	if(mgr->jpegdecoder  == NULL){
		LOGD("jpegdecoder null\n");
		return -1;
	}

    JpegDecoderSetDataSourceBuf(mgr->jpegdecoder, mgr->srcBuf,mgr->srcBufLen,mgr->scaleRatio,mgr->outputDataTpe);
    LOGD("srcBuf = %p,srcBufLen = %d\n",mgr->srcBuf,mgr->srcBufLen);

    //JpegDecoderSetDataSource(jpegdecoder, argv[1],scaleRatio,outputDataTpe);
    LOGD("ratio:%d type;%d\n",mgr->scaleRatio,mgr->outputDataTpe);


    mgr->imgFrame = JpegDecoderGetFrame(mgr->jpegdecoder);
    if(mgr->imgFrame == NULL){
        LOGW("JpegDecoderGetFrame fail\n");
        JpegDecoderDestory(mgr->jpegdecoder);
        return -1;
    }else{
            LOGD("JpegDecoderGetFrame successfully,imgFrame->mWidth = %d,imgFrame->mHeight = %d,imgFrame->mYuvData = %p,imgFrame->mYuvSize = %d\n",
                mgr->imgFrame->mWidth,mgr->imgFrame->mHeight,mgr->imgFrame->mYuvData,mgr->imgFrame->mYuvSize);
    }

    return 0;
}

void dump_mem(uint32_t * ptr ,int len){
int i=0;
for(i;i<len;i+=4){
LOGD("%x %x %x %x\n",ptr[i+0],ptr[i+1],ptr[i+2],ptr[i+3]);
}
LOGD("--\n");
}

#define CONF_KERNEL_IOMMU 1

#include <time.h>
#include <stdint.h>
long get_os_us(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert the time to milliseconds
    return  tv.tv_sec * 1000000 + tv.tv_usec ;
}

#define FPS_STAT_MAX 32
typedef struct {
    long tb[FPS_STAT_MAX];
    int cur;
    long last_fps;
} fps_mgr_t;
fps_mgr_t fps_mgr = {
    .cur = 0,
    .last_fps = -1,
};
long get_fps(void)
{
    fps_mgr_t * mgr = &fps_mgr;
    if(mgr->cur < FPS_STAT_MAX)//we ignore first loop and also ignore rollback case due to a long period
        return mgr->last_fps;//if <0 ,please ignore it
   else {
	int i=0;
	long b=0;
        long a = mgr->tb[(mgr->cur-1)%FPS_STAT_MAX];//cur
	for(i=2;i<FPS_STAT_MAX;i++){
	
        b = mgr->tb[(mgr->cur-i)%FPS_STAT_MAX]; //last
	if((a-b) > 1000000)
		break;
	}
        b = mgr->tb[(mgr->cur-i)%FPS_STAT_MAX]; //last
        long fps = (a - b) / (i-1);
        fps = (1000000*10 ) / fps;
        mgr->last_fps = fps;
        return fps;
    }
}
void put_fps_data(long t) //us
{
    fps_mgr_t * mgr = &fps_mgr;
    mgr->tb[mgr->cur%FPS_STAT_MAX] = t;
    mgr->cur++;//cur ptr to next
}




int udisp_main(int * running)
{
	int rv;
	int fb_width,fb_height;

	int loop=0;
	VoutRect rect;
	int enable = 0;
	int rotate = 0;
	deocde_jpg_mgr_t mgr={0};
	deocde_jpg_mgr_t *jmgr=&mgr;

	
	install_sig_handler();
	memset(&test_info, 0, sizeof(struct test_layer_info));
	test_info.buffer_num = 1;
	test_info.format = VIDEO_PIXEL_FORMAT_DEFAULT;
	test_info.flg_terminal=0;
	rv = parse_cmdline( &test_info);
	if(rv < 0) {
		LOGD("layer_request:parse_command\n");
		return -1;
	}

	system("dd if=/dev/zero of=/dev/fb0");

	if(mDispOutPort == NULL) {
		mDispOutPort = CreateVideoOutport(0);
		if(mDispOutPort == NULL){
			LOGD("CreateVideoOutport ERR\n");
			return -1;
		}
		rect.x = 0;
		rect.y = 0;
		rect.width = test_info.fb_width;
		rect.height = test_info.fb_height;
		mDispOutPort->init(mDispOutPort, enable, rotate, &rect);

		rect.x = 0;
		rect.y = 0;
		rect.width = mDispOutPort->getScreenWidth(mDispOutPort);
		rect.height = mDispOutPort->getScreenHeight(mDispOutPort);
		mDispOutPort->setRect(mDispOutPort,&rect);
	}

	fb_width = test_info.fb_width;
	fb_height = test_info.fb_height;

	pMemops = GetMemAdapterOpsS();
	SunxiMemOpen(pMemops);
	pVirBuf = (char*)SunxiMemPalloc(pMemops, fb_width * fb_height * 4 * test_info.buffer_num);
	pPhyBuf = (char*)SunxiMemGetPhysicAddressCpu(pMemops, pVirBuf);
	memset((void*)pVirBuf, 0x0, fb_width*fb_height*4*test_info.buffer_num);
	
	do {
		static int dec_issue_cnt=0;

		decode_jpg_create(jmgr);
		decode_jpg_prepare_data(jmgr,test_info.filename);//decode will free it, strange design, asymetic
		if(jmgr->srcBufLen <10){
			LOGI("loop:%d no data,retry\n",loop);
			usleep(10000);	
			continue;
		}
	
		decode_jpg(jmgr);	
		memcpy((void*)pVirBuf,jmgr->imgFrame->mYuvData,jmgr->imgFrame->mYuvSize);
		SunxiMemFlushCache(pMemops, (void*)pVirBuf, fb_width*fb_height*4*test_info.buffer_num);

		addrSetting();

		vparam.srcInfo.crop_x = 0;
		vparam.srcInfo.crop_y = 0;
		vparam.srcInfo.crop_w = fb_width;
		vparam.srcInfo.crop_h = fb_height;

		vparam.srcInfo.w = fb_width;
		vparam.srcInfo.h = fb_height;
		vparam.srcInfo.color_space = VIDEO_BT601;
		rBuf.isExtPhy = VIDEO_USE_EXTERN_ION_BUF;
#ifdef CONF_KERNEL_IOMMU
		rBuf.fd = SunxiMemGetBufferFd(pMemops, (void*) pVirBuf);
#endif


		mDispOutPort->queueToDisplay(mDispOutPort, vparam.srcInfo.w*vparam.srcInfo.h*4, &vparam, &rBuf);
		mDispOutPort->SetZorder(mDispOutPort, VIDEO_ZORDER_MIDDLE);
		mDispOutPort->setEnable(mDispOutPort, 1);

		decode_jpg_destroy(jmgr);
		put_fps_data(get_os_us());
		LOGI("%d fps %d(%d)\n",loop++,get_fps()/10,get_fps());

		} while(*running &&  !test_info.flg_terminal);

		test_info.flg_terminal=0;
	LOGI("go exit\n");

	mDispOutPort->setEnable(mDispOutPort, 0);

	SunxiMemPfree(pMemops, pVirBuf);
	SunxiMemClose(pMemops);

	pMemops = NULL;
	pVirBuf = NULL;

	close(test_info.fh);
	#if 0
	mDispOutPort->deinit(mDispOutPort);
	DestroyVideoOutport(mDispOutPort);
	mDispOutPort =NULL;
	#endif

	

	return 0;
}

static void addrSetting(void) {
    int width;
    int height;
    int format;

    width = test_info.fb_width;
    height = test_info.fb_height;
    format = test_info.format;

    vparam.srcInfo.format = test_info.format;
    switch(format){
        case VIDEO_PIXEL_FORMAT_NV21 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.v_phaddr = fbAddr[1];
            rBuf.u_phaddr = fbAddr[1];
            break;

        case VIDEO_PIXEL_FORMAT_NV12 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.v_phaddr = fbAddr[1];
            rBuf.u_phaddr = fbAddr[1];
            break;

        case VIDEO_PIXEL_FORMAT_YUV_MB32_420 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.v_phaddr = fbAddr[1];
            rBuf.u_phaddr = fbAddr[1];
            break;

        case VIDEO_PIXEL_FORMAT_YUV_PLANER_420 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            fbAddr[2] = (unsigned long)(fbAddr[0] + width * height * 5 / 4);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.u_phaddr = fbAddr[1];
            rBuf.v_phaddr = fbAddr[2];
            break;

        case VIDEO_PIXEL_FORMAT_YV12 :
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            fbAddr[2] = (unsigned long)(fbAddr[0] + width * height * 5 / 4);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.u_phaddr = fbAddr[2];
            rBuf.v_phaddr = fbAddr[1];
            break;
        case VIDEO_PIXEL_FORMAT_YUYV:
            fbAddr[0] = (unsigned long)pPhyBuf;
            fbAddr[1] = (unsigned long)(fbAddr[0] + width * height);
            fbAddr[2] = (unsigned long)(fbAddr[1] + width * height * 2);
            rBuf.y_phaddr = fbAddr[0];
            rBuf.u_phaddr = fbAddr[1];
            rBuf.v_phaddr = fbAddr[2];
            break;

        default :
            fbAddr[0] = (unsigned long)pPhyBuf;
            rBuf.y_phaddr = fbAddr[0];
            break;
    }
}
