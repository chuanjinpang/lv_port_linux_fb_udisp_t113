/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

//#define AUTO_TEST 1

static lv_obj_t * launcher_page[3];
static lv_obj_t * launcher_tab_icon;

lv_obj_t * launcher_cont ;
lv_obj_t * udisp_cont ;



LV_IMG_DECLARE(bg480480);

LV_IMG_DECLARE(icon_011);
LV_IMG_DECLARE(icon_014);
LV_IMG_DECLARE(icon_017);

int udisp_running=0;
pthread_t udisp_thread;

int udisp_main(int * running);


static void gesture_cb( lv_event_t * e)
{
lv_obj_t * obj= lv_event_get_target(e);
lv_dir_t dir= lv_indev_get_gesture_dir(lv_indev_get_act());
	printf("%s %d\n",__func__,dir);
	//add_disp(obj);
}


void * udisp_task(void * ctx){

do {

	
	printf("%s\n",__func__);
	//sleep(1);
	udisp_main(&udisp_running);


}
while(udisp_running);
	printf("%s exit\n",__func__);
	return NULL;

}
static void play_event_click_cb( lv_event_t * e);

static void udisp_event_click_cb( lv_event_t * e){

	printf("%s\n",__func__);

	play_event_click_cb(e);

}


lv_obj_t * gudisp_exit_btn;


void launcher_udisp_task(void)
{


	udisp_cont= lv_obj_create(lv_scr_act());
    //lv_obj_t *    launcher_bg = lv_img_create(udisp_cont);
    //lv_img_set_src(launcher_bg,&icon_011);
    lv_obj_set_size(udisp_cont,LV_HOR_RES_MAX,LV_VER_RES_MAX);
	lv_obj_t * btn=lv_btn_create(udisp_cont);
	lv_obj_set_size(btn,LV_VER_RES_MAX,LV_VER_RES_MAX/4);
	udisp_running=1;
	gudisp_exit_btn=btn;
	lv_obj_add_event_cb(btn,udisp_event_click_cb,LV_EVENT_CLICKED,NULL);
	pthread_create(&udisp_thread,NULL,udisp_task,NULL);
	lv_obj_add_event_cb(udisp_cont,gesture_cb,LV_EVENT_GESTURE,NULL);

}

static void udisp_delay_cb(lv_timer_t * task)
{

printf("%s\n",__func__);
lv_timer_set_repeat_count(task,0);
lv_event_send(gudisp_exit_btn,LV_EVENT_CLICKED,NULL);
}


static void play_event_click_cb( lv_event_t * e)
{
lv_obj_t * obj= lv_event_get_target(e);
	printf("%s\n",__func__);
	//add_disp(obj);
	if(udisp_running==0) {
		launcher_udisp_task();
		lv_obj_add_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);
	#ifdef AUTO_TEST
		lv_timer_create(udisp_delay_cb,5000,NULL);
	#endif
	}else {
		udisp_running=0;
		printf("%s wait udisp exit\n",__func__);
		pthread_join(udisp_thread,NULL);
		printf("%s wait udisp exit done\n",__func__);
		lv_obj_del(udisp_cont);
		lv_obj_clear_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);

	}
}


lv_obj_t * gicon_obj;


static void lv_example_grid_1(lv_obj_t * par,lv_img_dsc_t * icon)
{
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(par);
	lv_obj_set_style_bg_opa(cont,LV_OPA_0,LV_STATE_DEFAULT);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_t * label;
    lv_obj_t * obj;

    uint32_t i;
    for(i = 0; i < 6; i++) {
        uint8_t col = i % 3;
        uint8_t row = i / 3;

        obj = lv_imgbtn_create(cont);
		
	lv_imgbtn_set_src(obj,LV_IMGBTN_STATE_RELEASED,NULL,icon,NULL);
	lv_obj_add_flag(obj,LV_OBJ_FLAG_CLICKABLE);
	lv_obj_align(obj,LV_ALIGN_CENTER,0,0);
	lv_obj_add_event_cb(obj,play_event_click_cb,LV_EVENT_CLICKED,NULL);
	lv_obj_set_width(obj,icon->header.w);
        /*Stretch the cell horizontally and vertically too
         *Set span to 1 to make the cell 1 column/row sized*/
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col, 1,
                                  LV_GRID_ALIGN_CENTER, row, 1);

        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "c%d, r%d", col, row);
        lv_obj_center(label);
    }
}

static void benchmark_exit_event_click_cb( lv_event_t * e){
			lv_obj_t *	benchmark_cont = e->user_data;

			printf("%s %d \n",__func__,__LINE__);
			lv_obj_clear_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(benchmark_cont,LV_OBJ_FLAG_HIDDEN);
			//lv_anim_del(benchmark_cont, NULL);
			lv_anim_del_all();
			lv_obj_del(benchmark_cont);
			
			printf("%s %d \n",__func__,__LINE__);

}


static void benchmark_task_click_cb( lv_event_t * e){
static int b_running=0;
	lv_obj_t * obj= lv_event_get_target(e);
		printf("%s\n",__func__);
		//add_disp(obj);
		if(b_running==0) {
			//b_running=1;
			lv_obj_add_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);
			lv_obj_t *  benchmark_cont=lv_obj_create(lv_scr_act());
			lv_obj_set_size(benchmark_cont,LV_HOR_RES_MAX,LV_VER_RES_MAX);


	        lv_obj_t * benchmark_cont_header = lv_obj_create(benchmark_cont);
	        lv_obj_set_width(benchmark_cont_header,LV_HOR_RES_MAX);
			lv_obj_set_height(benchmark_cont_header,64);

			lv_obj_t * btn=lv_btn_create(benchmark_cont);
			lv_obj_set_size(btn,LV_HOR_RES_MAX,28);
			lv_obj_add_event_cb(btn,benchmark_exit_event_click_cb,LV_EVENT_CLICKED,benchmark_cont);

			
 	   		lv_obj_t * benchmark_cont_body = lv_obj_create(benchmark_cont);
	        lv_obj_set_width(benchmark_cont_body,LV_HOR_RES_MAX);			
    		lv_obj_set_height(benchmark_cont_body,LV_VER_RES_MAX-64);    		
			lv_obj_align(benchmark_cont_header,LV_ALIGN_TOP_LEFT,0,0);
   			lv_obj_align(benchmark_cont_body,LV_ALIGN_BOTTOM_LEFT,0,0);
			//lv_demo_benchmark(benchmark_cont_body);
			//lv_demo_widgets(benchmark_cont_body);
			//lv_example_calendar_1(benchmark_cont_body);
			//lv_demo_music(benchmark_cont_body);
			lv_example_meter_3(benchmark_cont_body);
			//lv_obj_clear_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);

		}else {
			b_running=0;
			lv_obj_clear_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);
	
		}

}

static void dmo_task_click_cb( lv_event_t * e){
static int b_running=0;
	lv_obj_t * obj= lv_event_get_target(e);
		printf("%s\n",__func__);
		//add_disp(obj);
		if(b_running==0) {
			//b_running=1;
			lv_obj_add_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);
			lv_obj_t *  benchmark_cont=lv_obj_create(lv_scr_act());
			lv_obj_set_size(benchmark_cont,LV_HOR_RES_MAX,LV_VER_RES_MAX);


	        lv_obj_t * benchmark_cont_header = lv_obj_create(benchmark_cont);
	        lv_obj_set_width(benchmark_cont_header,LV_HOR_RES_MAX);
			lv_obj_set_height(benchmark_cont_header,64);

			lv_obj_t * btn=lv_btn_create(benchmark_cont);
			lv_obj_set_size(btn,LV_HOR_RES_MAX,28);
			lv_obj_add_event_cb(btn,benchmark_exit_event_click_cb,LV_EVENT_CLICKED,benchmark_cont);

			
 	   		lv_obj_t * benchmark_cont_body = lv_obj_create(benchmark_cont);
	        lv_obj_set_width(benchmark_cont_body,LV_HOR_RES_MAX);			
    		lv_obj_set_height(benchmark_cont_body,LV_VER_RES_MAX-64);    		
			lv_obj_align(benchmark_cont_header,LV_ALIGN_TOP_LEFT,0,0);
   			lv_obj_align(benchmark_cont_body,LV_ALIGN_BOTTOM_LEFT,0,0);
			//lv_demo_benchmark(benchmark_cont_body);
			//lv_demo_widgets(benchmark_cont_body);
			lv_example_calendar_1(benchmark_cont_body);
			//lv_demo_music(benchmark_cont_body);
			//lv_example_meter_3(benchmark_cont_body);
			//lv_obj_clear_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);

		}else {
			b_running=0;
			lv_obj_clear_flag(launcher_cont,LV_OBJ_FLAG_HIDDEN);
	
		}


}


typedef void ( * icon_task_cb_t )( lv_event_t * e);

static void lv_example_grid_demo(lv_obj_t * par,int tb_cnt,lv_img_dsc_t ** icon_tb,icon_task_cb_t * cb_tb)
{
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1),LV_GRID_FR(1),LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(par);
	lv_obj_set_style_bg_opa(cont,LV_OPA_0,LV_STATE_DEFAULT);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_t * label;
    lv_obj_t * obj;

    uint32_t i;
    for(i = 0; i < tb_cnt; i++) {
        uint8_t col = i % 3;
        uint8_t row = i / 3;

        obj = lv_imgbtn_create(cont);
		if(i==0)
			gicon_obj =obj;
	lv_imgbtn_set_src(obj,LV_IMGBTN_STATE_RELEASED,NULL,icon_tb[i],NULL);
	lv_obj_add_flag(obj,LV_OBJ_FLAG_CLICKABLE);
	lv_obj_align(obj,LV_ALIGN_CENTER,0,0);
	lv_obj_add_event_cb(obj,cb_tb[i],LV_EVENT_CLICKED,NULL);
	lv_obj_set_width(obj,icon_tb[i]->header.w);
        /*Stretch the cell horizontally and vertically too
         *Set span to 1 to make the cell 1 column/row sized*/
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col, 1,
                                  LV_GRID_ALIGN_CENTER, row, 1);

        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "c%d, r%d", col, row);
        lv_obj_center(label);
    }
}



static void launcher_icon_create(lv_obj_t * par)
{
    int i;
#if SHADOW_LABEL_EN
    lv_obj_t * shadow_label;
#endif
    lv_obj_t * t1,*t2,*t3;
    if(launcher_tab_icon == NULL)
    {
	lv_obj_t *lab,* tab_btns;
        launcher_tab_icon = lv_tabview_create(par,LV_DIR_TOP,50);
		lv_obj_set_style_bg_opa(launcher_tab_icon,LV_OPA_0,LV_STATE_DEFAULT);
        t1=lv_tabview_add_tab(launcher_tab_icon,"A PAGE");
        t2=lv_tabview_add_tab(launcher_tab_icon,"B PAGE");
        t3=lv_tabview_add_tab(launcher_tab_icon,"C PAGE");
        tab_btns=lv_tabview_get_tab_btns(launcher_tab_icon);
	lv_obj_add_flag(tab_btns,LV_OBJ_FLAG_HIDDEN);
	lab=lv_label_create(t1);
	lv_label_set_text(lab,"1");
	lab=lv_label_create(t2);
	lv_label_set_text(lab,"2");
	lab=lv_label_create(t3);
	lv_label_set_text(lab,"3");
	lv_obj_set_size(launcher_tab_icon,lv_pct(100),lv_pct(100));
		{
		lv_img_dsc_t * icon_tb[]={
			&icon_011,
			&icon_014,
			&icon_017
			};
		icon_task_cb_t cb_tb[]={
			play_event_click_cb,
				benchmark_task_click_cb,
				dmo_task_click_cb
			};

    	lv_example_grid_demo(t1,3,icon_tb,cb_tb);
		}
    	lv_example_grid_1(t2,&icon_014);
    	lv_example_grid_1(t3,&icon_017);

    }
#if 0
    for(i = 0;i < MAX_ICON_NUM - i_launcher_sta_icon;i++)
    {
        if(icon_img[i] == NULL)
        {
            icon_img[i] = lv_imgbtn_create(lv_tabview_get_tab(launcher_tab_icon,i/8),NULL);
            lv_obj_set_event_cb(icon_img[i],icon_click_event_cb);
        }
        lv_imgbtn_set_src(icon_img[i],LV_BTN_STATE_RELEASED,img_array_l[move_icon_id[i/8][i%8]]);
        lv_imgbtn_set_src(icon_img[i],LV_BTN_STATE_PRESSED,img_array_l[move_icon_id[i/8][i%8]]);

        if(icon_name[i] == NULL)
        {
            icon_name[i] = lv_label_create(lv_tabview_get_tab(launcher_tab_icon,i/8),NULL);
            //lv_obj_set_event_cb(icon_name[i],icon_click_event_cb);
        }
        lv_obj_set_style_local_text_font(icon_name[i],LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,&IconFont12);
        lv_label_set_text(icon_name[i],icon_name_tal[i_icon_language][move_icon_id[i/8][i%8]]);

#if 1
        lv_obj_set_style_local_text_color(icon_name[i],LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_WHITE);
#endif
        if((i%8) == 0)
        {
            lv_obj_align(icon_img[i],lv_tabview_get_tab(launcher_tab_icon,i/8),LV_ALIGN_TOP_LEFT, lp,icw*2);
        }
        else if((i%8) == 4)
        {
            lv_obj_align(icon_img[i],icon_img[ i - 4],LV_ALIGN_OUT_BOTTOM_MID, 0,icw*3);
        }
        else
        {
            lv_obj_align(icon_img[i],icon_img[ i - 1], LV_ALIGN_OUT_RIGHT_MID, icw,0);
        }
        lv_obj_align(icon_name[i],icon_img[i],LV_ALIGN_OUT_BOTTOM_MID, 0,10);
        icon_pos[i].x = lv_obj_get_x(icon_img[i]);
        icon_pos[i].y = lv_obj_get_y(icon_img[i]);
#if SHADOW_LABEL_EN
    shadow_label = lv_label_create(par,icon_name[i]);
    lv_obj_set_style_local_text_color(shadow_label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
    lv_obj_align(shadow_label,icon_name[i],LV_ALIGN_CENTER,-1,-1);
#endif
    }

    lv_obj_set_event_cb(launcher_tab_icon,tab_move_event_cb);
#endif
}
#define LED_ICON_PAGE_DIS  40
static lv_obj_t * led_icon_page[3];
static int i_launcher_page = 3;

static void launcher_led_create(lv_obj_t * par)
{
    int i;
	#define LED_SIZE  8
    lv_obj_t * cont = lv_obj_create(par);
	lv_obj_set_style_bg_opa(cont,LV_OPA_0,LV_STATE_DEFAULT);
    lv_obj_set_size(cont,lv_pct(100),lv_pct(36));
    for(i=0;i<i_launcher_page;i++)
    {
        if(i == 0)
        {
            if(led_icon_page[i] == NULL)
            {
                led_icon_page[i] = lv_led_create(cont);
            }
            lv_obj_set_size(led_icon_page[0],LED_SIZE,LED_SIZE);
            lv_led_set_brightness(led_icon_page[0],LV_LED_BRIGHT_MAX);
	    lv_led_on(led_icon_page[0]);
        }
        else
        {
            if(led_icon_page[i] == NULL)
            {
                led_icon_page[i] = lv_led_create(cont);
            }
            lv_obj_set_size(led_icon_page[i],LED_SIZE,LED_SIZE);
            lv_led_set_brightness(led_icon_page[i],LV_LED_BRIGHT_MIN);
	    lv_led_off(led_icon_page[i]);
        }
        lv_obj_align(led_icon_page[i],LV_ALIGN_CENTER,(i-1)*LED_ICON_PAGE_DIS,2);
    }
}



static void led_icon_page_handle(void)
{
    int i;
    lv_tabview_t * tb = (lv_tabview_t *) launcher_tab_icon;
    lv_obj_t * cont = lv_tabview_get_content(launcher_tab_icon);
    lv_coord_t cx=lv_obj_get_scroll_x(cont);

    for(i=0;i<i_launcher_page;i++)
    {
	    if(i == tb->tab_cur){
		    lv_led_on(led_icon_page[i]);
		}
	else {
		lv_led_off(led_icon_page[i]);
	}
    }
}



static void user_task_cb(lv_timer_t * task)
{


    led_icon_page_handle();
}


static void delay_cb(lv_timer_t * task)
{

printf("%s\n",__func__);
//lv_timer_set_repeat_count(task,0);
lv_event_send(gicon_obj,LV_EVENT_CLICKED,NULL);
}





void launcher_widgets(void)
{
    int i;
	
	launcher_cont=lv_obj_create(lv_scr_act());
	lv_obj_set_size(launcher_cont,LV_HOR_RES_MAX,LV_VER_RES_MAX);
#if 1
    lv_obj_t *  launcher_bg = lv_img_create(launcher_cont);
    lv_img_set_src(launcher_bg,&bg480480);
    lv_obj_set_size(launcher_bg,LV_HOR_RES_MAX,LV_VER_RES_MAX);
#endif

#if 1
    //lv_obj_set_style_local_image_recolor(launcher_bg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    //lv_obj_set_style_local_image_recolor_opa(launcher_bg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_30);
    for(i=0;i<3;i++)
    {
        launcher_page[i] = lv_obj_create(launcher_cont);
		lv_obj_set_style_bg_opa(launcher_page[i],LV_OPA_0,LV_STATE_DEFAULT);
        lv_obj_set_width(launcher_page[i],LV_HOR_RES_MAX);
    }
    lv_obj_set_height(launcher_page[0],(LV_VER_RES_MAX>>5) + 10);
    lv_obj_set_height(launcher_page[2],(LV_VER_RES_MAX/5) - 10);
    lv_obj_set_height(launcher_page[1],LV_VER_RES_MAX - lv_obj_get_height(launcher_page[0]) - lv_obj_get_height(launcher_page[2]));

    lv_obj_align(launcher_page[0],LV_ALIGN_TOP_LEFT,0,0);
    lv_obj_align(launcher_page[2],LV_ALIGN_BOTTOM_LEFT,0,0);
    lv_obj_align_to(launcher_page[1],launcher_page[0],LV_ALIGN_OUT_BOTTOM_LEFT,0,0);

    launcher_icon_create(launcher_page[1]);
    launcher_led_create(launcher_page[2]);
    lv_timer_create(user_task_cb, 20, NULL);
#if 0
    launcher_taskbar_create(launcher_page[0]);
    launcher_static_icon_create(launcher_page[2]);
#endif
    lv_obj_add_event_cb(launcher_cont,gesture_cb,LV_EVENT_GESTURE,NULL);
#ifdef AUTO_TEST
	lv_timer_create(delay_cb,10000,NULL);
#endif

#endif
}
