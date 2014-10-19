/*
* File name:        osdFont.c
* Version:          1.0
* Date:             2014.06.09
* Description:      gennerate yuv 422 or yuv 420 by osd font.     
* Author:           Liwenyan
* Email:            suma.lwy@gmail.com
*/
  
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
  
#ifndef  FALSE
#define  FALSE                     0
#endif

#ifndef  TRUE
#define  TRUE                      1
#endif


#define FRAME_WIDTH             (100) // (16*2 + 2*2)
#define FRAME_HEIGHT            (100) // (16*2 + 2*2)
#define YUV_422_FRAME_SIZE      (FRAME_WIDTH*FRAME_HEIGHT*2)
#define YUV_420_FRAME_SIZE      (FRAME_WIDTH*FRAME_HEIGHT*3/2)
#define IN_FILENAME             "in.yuv"
#define OUT_FILENAME            "out.yuv"

#if 1
#define OSD_FILE_NAME_SIZE      "font/en_12x24.dat"
#define FONT_WIDTH              (16)
#define FONT_HEIGHT             (24)
#define FONT_NUM                (FONT_WIDTH*FONT_HEIGHT/8)
#else
#define OSD_FILE_NAME_SIZE      "font/gb2312_24x24.dat"
#define FONT_WIDTH              (24)
#define FONT_HEIGHT             (24)
#define FONT_NUM                (FONT_WIDTH*FONT_HEIGHT/8)
#endif

enum
{
    CENTER_BORDER = 2,
    LEFT_BORDER =0x04,
    RIGHT_BORDER =0x08,
    UP_BORDER  = 0x010,
    DOWN_BORDER = 0x20
};

typedef struct
{
    uint8_t y;
    uint8_t u;
    uint8_t v;
}yuv_data_t;

const yuv_data_t font_yuv ={0x96, 0x10, 0x16};
const yuv_data_t border_yuv ={0x16, 0xA0, 0xC6};
  
const unsigned char pic_table_yuv422[] = {
    0x80, 0x10, 0x80, 0x10
};

static uint8_t *create_yuv422_frame(int windth, int height, int y, int u, int v)
{
    int i;
    uint8_t *buf = NULL;
    uint8_t *cur_buf = NULL;
    uint8_t yuv[4];

    if( (windth <= 0) || (height <= 0))
    {
        return NULL;
    }

    buf = malloc(windth*height*2);
    memcpy(yuv, pic_table_yuv422, sizeof(pic_table_yuv422));
    yuv[0] = u;
    yuv[1] = y;
    yuv[2] = v;
    yuv[3] = y;

    cur_buf = buf;
    for(i = 0; i<windth*height*2; i += sizeof(yuv))
    {
        memcpy(cur_buf, yuv, sizeof(yuv));
        cur_buf += sizeof(yuv);
    }

    return buf;
}

static uint8_t *create_yuv420_frame(int windth, int height, int y, int u, int v)
{
    int i;
    uint8_t *buf = NULL;
    uint8_t *cur_buf = NULL;
    uint8_t uv[2];

    if( (windth <= 0) || (height <= 0))
    {
        return NULL;
    }

    buf = malloc(YUV_420_FRAME_SIZE);

    memset(buf, y&0xFF, (YUV_420_FRAME_SIZE*2/3));
    cur_buf = buf + (YUV_420_FRAME_SIZE*2/3);

    uv[0] = u&0xFF;
    uv[1] = v&0xFF;
    for(i = 0; i<(YUV_420_FRAME_SIZE/3); i += 2)
    {
        memcpy(cur_buf, uv, 2);
        cur_buf += 2;
    }

    return buf;
}

static void destory_yuv422_frame(uint8_t *buf)
{
    free(buf);
}

int get_point_status(const unsigned char font[], int x, int y)
{
    uint8_t p8 = 0;
    uint16_t mask8; // for reading hzk16 dots
    int line_num;
    int status = 0;
    int pos_x = 0;

    pos_x = x%8;
    mask8 = 0x80;
    p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*y + x/8));
    for (line_num = 0; line_num < pos_x; line_num++)    // dots in a line
    {        
        mask8 >>= 1;  /* 循环移位取数据 */
        if (mask8 == 0)
        {
            mask8 = 0x80;
        }
    }

    if((p8 & mask8))
    {
        //printf("\n p8:%x, mask8:%x \n", p8, mask8);
        return 1;
    }

    return 0;
}

int get_border_status(const unsigned char font[], int x, int y)
{
    uint8_t p8 = 0, cur_p8 = 0;
    uint8_t up_p8 = 0, down_p8 = 0;
    uint8_t mask8; // for reading hzk16 dots
    uint8_t next_mask8 = 0, last_mask8 = 0; // for reading hzk16 dots
    int line_num;
    int up_status = 0;
    int down_status = 0;
    int left_status = 0;
    int right_status = 0;
    int status = 0;
    int pos_x = 0;

    pos_x = x%8;
    mask8 = 0x80;
    last_mask8 = 0x01;
    next_mask8 = (0x80>>1);
    p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*y + x/8));
    for (line_num = 0; line_num < pos_x; line_num++)    // dots in a line
    {
        last_mask8 = mask8;
        mask8 = next_mask8;
        next_mask8 = mask8 >> 1;  /* 循环移位取数据 */
        if (next_mask8 == 0)
        {
            next_mask8 = 0x80;
        }
    }

    if(p8 & mask8)
    {
        /* add border */
        if(x == 0){
            status |= LEFT_BORDER;
        }
        else if (x == (FONT_WIDTH -1) ){
            status |= RIGHT_BORDER;
        }
        if(y == 0){
            status |= UP_BORDER;
        }
        else if(y == (FONT_WIDTH -1)){
            status |= DOWN_BORDER;
        }
        if(0 != status){
            //printf("\n status:%d \n", status);
            return status;
        }
    }
    else
    {
        if(0 == x){
            last_mask8 = 0;
        }
        else if((FONT_WIDTH - 1) == x){
            next_mask8 = 0;
        }

        if(0 == y){
            up_p8 = 0;
            down_p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*(y + 1) + x/8));
        }
        else if((FONT_WIDTH- 1) == y){
            up_p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*(y - 1) + x/8));
            down_p8 = 0;
        }
        else{
            up_p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*(y - 1) + x/8));
            down_p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*(y + 1) + x/8));
        }
        up_status = (up_p8 & mask8);
        down_status = (down_p8 & mask8);

        if(0 == last_mask8){
            left_status = 0;
        }
        else{
            uint8_t last_p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*y + x/8 - 1));
            cur_p8 = (pos_x != 0) ? p8: last_p8;
            left_status = (cur_p8 & last_mask8);
        }
        if(0 == next_mask8){
            right_status = 0;
        }
        else{
            uint8_t next_p8 = (uint8_t)(*(uint8_t *)(font +(FONT_WIDTH/8)*y + x/8 + 1));
            cur_p8 = (pos_x != (8 -1)) ? p8: next_p8;
            right_status = (cur_p8 & next_mask8);
        }

        if( 
           (up_status) ||
           (down_status) ||
           (left_status) ||
           (right_status)
        )
        {
            return CENTER_BORDER;
        }
    }

    return 0;
}

static void set_yuv420_color(char *ptr_frame, int x, int y, const yuv_data_t *in_yuv)
{
    char *offsetY = NULL;
    char *offsetU = NULL;
    char *offsetV = NULL;

    offsetY = ptr_frame;
    offsetU = offsetY + (FRAME_WIDTH*FRAME_HEIGHT);
    offsetV = offsetU + (FRAME_WIDTH*FRAME_HEIGHT)/4;
    *(offsetY + (FRAME_WIDTH)*(2*y) + (2*x)) = in_yuv->y; /* 2*x yuv422 mode */
    *(offsetY + (FRAME_WIDTH)*(2*y) + (2*x+1)) = in_yuv->y; /* 2*x yuv422 mode */
    *(offsetY + (FRAME_WIDTH)*(2*y+1) + (2*x)) = in_yuv->y; /* 2*x yuv422 mode */
    *(offsetY + (FRAME_WIDTH)*(2*y+1) + (2*x+1)) = in_yuv->y; /* 2*x yuv422 mode */
    *(offsetU + (FRAME_WIDTH/2)*(y)+ (x)) = in_yuv->u; /* 2*x yuv422 mode */
    *(offsetV + (FRAME_WIDTH/2)*(y)+ (x)) = in_yuv->v; /* 2*x yuv422 mode */

    return;
}

/* font 2 yuv422 or 420 */
char *font2yuv(char *ptr_frame,const unsigned char font[], int font_len, int startx,int starty,int color)
{
    char *offsetY = NULL;
    char *offsetU = NULL;
    char *offsetV = NULL;
    uint32_t p48, mask48; // for reading hzk16 dots
    int status = 0;
    int x=0,y=0,colum=0,line_num=0;
    int border_x = 0, border_y = 0;
    const yuv_data_t *in_yuv;

    assert( ptr_frame != NULL );
    /*yuv 地址的设置 */
    offsetY = ptr_frame;
    offsetU = offsetY + (FRAME_WIDTH*FRAME_HEIGHT);
    offsetV = offsetU + (FRAME_WIDTH*FRAME_HEIGHT)/4;
    y = starty;
    for (line_num = 0; line_num < FONT_HEIGHT; line_num++) // line dots per char
    {
        x = startx;
        for (colum = 0; colum < FONT_WIDTH; colum++)    // dots in a line
        {
            if ((status = get_point_status(font, colum, line_num)))
            {
                if(1 == status)
                {
                    in_yuv = &(font_yuv);
                    set_yuv420_color(ptr_frame, x, y, in_yuv);
                }
            }
            if ((status = get_border_status(font, colum, line_num)))
            {
                in_yuv = &(border_yuv);
                if(CENTER_BORDER == status)
                {
                    border_x = x;
                    border_y = y;
                    set_yuv420_color(ptr_frame, border_x, border_y, in_yuv);
                }
                else
                {
                    if(LEFT_BORDER == (status&LEFT_BORDER))
                    {
                        border_x = x-1;
                        border_y = y;
                        set_yuv420_color(ptr_frame, border_x, border_y, in_yuv);
                    }
                    if(RIGHT_BORDER == (status&RIGHT_BORDER))
                    {
                        border_x = x+1;
                        border_y = y;
                        set_yuv420_color(ptr_frame, border_x, border_y, in_yuv);
                    }
                    if(UP_BORDER == (status&UP_BORDER))
                    {
                        border_x = x;
                        border_y = y-1;
                        set_yuv420_color(ptr_frame, border_x, border_y, in_yuv);
                    }
                    if(DOWN_BORDER == (status&DOWN_BORDER))
                    {
                        border_x = x;
                        border_y = y+1;
                        set_yuv420_color(ptr_frame, border_x, border_y, in_yuv);
                    }
                }
            }
            x++;
        }
        y++;
    }

    return (char *)ptr_frame;
}

#define MAX_TXT_LEN   (64)
int main(int argc,char * argv[])
{
    int ret = 0;
    FILE *in_file,*out_file;
    FILE *file_ch = NULL;

    char *frame_buffer = NULL;
    frame_buffer = (char *)malloc(YUV_420_FRAME_SIZE);
    char str_table[128][FONT_NUM];
    char str[128];
    uint8_t string_end = FALSE;
    uint8_t osd_txt[FONT_NUM];
    int ind;
    uint8_t a1,a2;
    uint32_t offset;
    char *cur_ptr = &str_table[0][0];
    char *file_name;
    int i, j;

    file_ch = fopen(OSD_FILE_NAME_SIZE, "r");
    memset(str, 0, sizeof(str));
    memset(str_table, 0, sizeof(str_table));
    strcpy(str, argv[1]);
    offset = atoi(str);

    printf("\n offset:%d\n", offset);
    offset *= FONT_NUM;//((a1 - 0xa1)*94 + a2 - 0xa1)* (FONT_NUM);

    fseek(file_ch, offset, SEEK_SET);
    fread(osd_txt, FONT_NUM, 1, file_ch);
    memcpy(cur_ptr, osd_txt, FONT_NUM);
    for(i = 0; i<FONT_NUM; i++)
    {
        //printf("\n osd_txt[%d]:%x \t", i, osd_txt[i]);
    }
    cur_ptr += FONT_NUM;
    ind = 1;

    //数据转换
    frame_buffer = create_yuv420_frame(FRAME_WIDTH, FRAME_HEIGHT, 0xFA, 0x7D, 0x7E);
    //frame_buffer = create_yuv422_frame(FRAME_WIDTH, FRAME_HEIGHT, 0xFA, 0x7D, 0x7E);
    font2yuv(frame_buffer, &str_table[0][0], ind, 1, 1, 1);

    //write frame file 把数据写回
    out_file = fopen(OUT_FILENAME, "w");
    if(out_file == NULL)
    {
        printf("open in file error!\n");
    }
    ret = fwrite(frame_buffer, YUV_420_FRAME_SIZE, 1, out_file);
    // ret = fwrite(frame_buffer, 16*16*2,1,out_file);
    if(ret != 1)
    {
        printf("ret = %d\n");
        printf("fwrite file error!\n");
    }
    fclose(out_file);
    free(frame_buffer);

    printf("Done!\n");
    return 0;
}

