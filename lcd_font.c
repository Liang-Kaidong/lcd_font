/******************************************************************************************************************
File name:lcd_font.c
Author:Liang Kaidong
Version:V_1.0
Build date: 2025-05-21
Description:This file provides a series of functions for graphic drawing and text rendering on an LCD screen. The
            main functions include initializing and managing LCD devices and font resources, supporting the loading
            of TrueType font files; offering basic drawing capabilities such as clearing the screen,drawing pixels, 
            lines, rectangles, and rounded rectangles; having text processing abilities that support decoding UTF-8
            encoded text, calculating text width and height, and rendering text; allowingthe setting of font size 
            and enabling the rendering of text with text boxes.
Others:Usage requires preservation of original author attribution.
Log:1.Optimize code comments and remove some useless code.
******************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

/* 基于 TrueType 字体的开源库 */
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/* 字库头文件 */
#include "lcd_font.h"

/* LCD 设备结构体 */
typedef struct {
    int fd;         /* LCD 设备文件的文件描述符，对设备文件进行读写操作 */
    uint16_t *mp;   /* 指向 LCD 设备内存映射区域的指针，可以直接操作 LCD 屏幕的像素数据 */
    int width;      /* 屏幕宽度（长） */
    int height;     /* 屏幕高度（宽） */
} LcdDevice;

/* 全局变量 */
static LcdDevice *lcd = NULL;               /* 存储当前 LCD 设备的信息 */
static stbtt_fontinfo font;                 /* 存储字体信息 */
static unsigned char *font_buffer = NULL;   /* 存储字体文件的内存缓冲区 */
static int font_size = 24;                  /* 字体大小初始值为 24 */

/* 初始化 LCD 设备 */ 
static LcdDevice* init_lcd_device(const char *lcd_path, int width, int height) {
    LcdDevice *device = (LcdDevice*)malloc(sizeof(LcdDevice));      /* 使用 malloc 函数为 LcdDevice 结构体分配内存 */
    if (!device) {
        perror("malloc");
        return NULL;
    }

    /* 打开 LCD 设备 */
    device->fd = open(lcd_path, O_RDWR);
    if (device->fd == -1) {
        perror("open");
        free(device);
        return NULL;
    }

    /* 内存映射 */
    device->mp = (uint16_t*)mmap(0, width * height * 2, PROT_READ | PROT_WRITE, MAP_SHARED, device->fd, 0);
    if (device->mp == MAP_FAILED) {
        perror("mmap");
        close(device->fd);
        free(device);
        return NULL;
    }

    /* 设置设备参数并返回 */
    device->width = width;
    device->height = height;
    return device;
}

/* 释放 LCD 设备资源 */ 
static void free_lcd_device(LcdDevice *device) {
    if (device) {
        if (device->mp != MAP_FAILED) {
            munmap(device->mp, device->width * device->height * 2);
        }
        if (device->fd != -1) {
            close(device->fd);
        }
        free(device);
    }
}

/* 
* UTF-8 解码函数
* str：指向 UTF-8 编码字符串的指针，作为函数的输入参数。
* codepoint：指向整数的指针，用于存储解码后的 Unicode 码点。
*/ 
static int decode_utf8(const char *str, int *codepoint) {   
    unsigned char c = (unsigned char)*str;  /* c：用于存储当前字符的第一个字节 */
    int len = 1;    /* len：用于存储当前字符的字节长度，初始化为 1，因为单字节 UTF-8 字符占用 1 个字节 */
    
    /* UTF-8 编码规则进行解码 */
    if (c < 0x80) {                     /* 单字节字符（ASCII 字符） */
        *codepoint = c;
    } else if ((c & 0xE0) == 0xC0) {    /* 双字节字符 */
        *codepoint = ((c & 0x1F) << 6) | (str[1] & 0x3F);
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {    /* 三字节字符 */
        *codepoint = ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        len = 3;
    } else if ((c & 0xF8) == 0xF0) {    /* 四字节字符 */
        *codepoint = ((c & 0x07) << 18) | ((str[1] & 0x3F) << 12) | 
                      ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        len = 4;
    } else {
        *codepoint = 0xFFFD;            /* 无效字符 */ 
    }
    
    return len;  /* 返回当前字符的字节长度 */
}

/* 初始化字库 */ 
int lcd_init(const char *lcd_path, const char *font_path) {

    /* 释放现有资源（如果有） */ 
    lcd_cleanup();

    /* 初始化 LCD 设备 */ 
    lcd = init_lcd_device(lcd_path, 1024, 600);  /* 假设屏幕分辨率为 1024x600 */
    if (!lcd) {
        return -1;
    }

    /* 读取字体文件 */ 
    FILE *font_file = fopen(font_path, "rb");
    if (!font_file) {
        perror("无法打开字体文件.");
        lcd_cleanup();
        return -1;
    }
    
    fseek(font_file, 0, SEEK_END);      /* 使用 fseek 和 ftell 确定字体文件的大小 */
    long font_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font_buffer = (unsigned char*)malloc(font_size);
    if (!font_buffer) {
        perror("malloc");
        fclose(font_file);
        lcd_cleanup();
        return -1;
    }
    fread(font_buffer, 1, font_size, font_file);    /* 将字体文件内容读取到 font_buffer 中 */
    fclose(font_file);  /* 关闭文件 */

    /* 初始化字体信息 */ 
    if (!stbtt_InitFont(&font, font_buffer, 0)) {
        perror("无法初始化字体.");
        lcd_cleanup();
        return -1;
    }

    return 0;
}

/* 清理资源，防内存泄漏 */ 
void lcd_cleanup(void) {
    if (font_buffer) {          /* 检查 font_buffer 指针是否不为 NULL */
        free(font_buffer);
        font_buffer = NULL;     /* 避免成为悬空指针 */
    }
    if (lcd) {                  /* 检查 lcd 指针是否不为 NULL */
        free_lcd_device(lcd);
        lcd = NULL;             /* 避免成为悬空指针 */
    }
}

/* 清空屏幕 */ 
void lcd_clear(color_t color) {     
    if (!lcd) return;   /* LCD 未初始化，返回 */
    
    for (int i = 0; i < lcd->width * lcd->height; i++) {
        lcd->mp[i] = color;  /* 将整个 LCD 屏幕填充为指定颜色 */
    }
}

/* 设置字体大小 */ 
void lcd_set_font_size(int size) {
    /*
    * 将全局变量 font_size 的值更新为传入的参数 size。
    * font_size 会影响后续文本宽度、高度的计算以及文本渲染的实际效果。
    */
    font_size = size;   
}

/* 绘制像素点 */ 
void lcd_draw_pixel(int x, int y, color_t color) {
    if (!lcd) return;
    if (x >= 0 && x < lcd->width && y >= 0 && y < lcd->height) {
        lcd->mp[y * lcd->width + x] = color;
    }
}

/* 绘制直线（Bresenham算法） */ 
void lcd_draw_line(int x1, int y1, int x2, int y2, color_t color) {
    if (!lcd) return;
    
    /* 
    * 功能：在两点 (x1, y1) 和 (x2, y2) 之间绘制一条直线。
    * 参数：x1、y1：直线起点坐标。x2、y2：直线终点坐标。color：直线颜色。
    * 逻辑：先计算两点间的水平和垂直距离 dx、dy，以及 x、y 方向的步进值 sx、sy。
    *      接着使用 err 变量来决定下一个像素点的位置，在循环中不断调用
    *      lcd_draw_pixel 绘制像素点，直到到达终点。 
    */
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        lcd_draw_pixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/* 绘制矩形 */ 
void lcd_draw_rectangle(int x, int y, int width, int height, color_t color) {
    if (!lcd) return;
    
    /* 
    * 功能：绘制一个空心矩形。
    * 参数：x、y：矩形左上角坐标。width、height：矩形的宽度和高度。color：矩形边框颜色。
    * 逻辑：调用 lcd_draw_line 函数分别绘制矩形的四条边。
    */

    /* 绘制四条边 */ 
    lcd_draw_line(x, y, x + width - 1, y, color);                               /* 上边 */ 
    lcd_draw_line(x, y + height - 1, x + width - 1, y + height - 1, color);     /* 下边 */
    lcd_draw_line(x, y, x, y + height - 1, color);                              /* 左边 */
    lcd_draw_line(x + width - 1, y, x + width - 1, y + height - 1, color);      /* 右边 */
}

/* 绘制填充矩形 */ 
void lcd_draw_filled_rectangle(int x, int y, int width, int height, color_t color) {
    if (!lcd) return;
    
    /* 
    * 功能：绘制一个填充矩形。
    * 参数：x、y：矩形左上角坐标。width、height：矩形的宽度和高度。color：矩形填充颜色。
    * 逻辑：使用两层循环遍历矩形区域内的所有像素点，检查坐标是否在屏幕范围内，
    *      若在则直接操作屏幕缓冲区设置像素颜色。
    */
    for (int j = y; j < y + height; j++) {
        for (int i = x; i < x + width; i++) {
            if (i >= 0 && i < lcd->width && j >= 0 && j < lcd->height) {
                lcd->mp[j * lcd->width + i] = color;
            }
        }
    }
}

/* 绘制圆角矩形 */ 
void lcd_draw_rounded_rectangle(int x, int y, int width, int height, int radius, color_t color) {
    if (!lcd) return;
    
    /* 
    * 功能：绘制一个空心圆角矩形。
    * 参数：x、y：矩形左上角坐标。width、height：矩形的宽度和高度。radius：圆角半径。color：矩形边框颜色。
    * 逻辑：先确保圆角半径不超过矩形宽度或高度的一半。接着调用 lcd_draw_line 绘制四条边，
    *      再通过两层循环和圆的方程判断像素点是否在圆弧边缘，若在则调用 lcd_draw_pixel 
    *      绘制四个角的圆弧。
    */

    /* 确保半径不超过宽度或高度的一半 */ 
    if (radius > width/2) radius = width/2;
    if (radius > height/2) radius = height/2;
    
    /* 绘制四条边 */ 
    lcd_draw_line(x + radius, y, x + width - radius, y, color);                             /* 上边 */ 
    lcd_draw_line(x + radius, y + height - 1, x + width - radius, y + height - 1, color);   /* 下边 */ 
    lcd_draw_line(x, y + radius, x, y + height - radius, color);                            /* 左边 */ 
    lcd_draw_line(x + width - 1, y + radius, x + width - 1, y + height - radius, color);    /* 右边 */ 
    
    /* 绘制四个角的圆弧 */ 
    for (int i = 0; i <= radius; i++) {
        for (int j = 0; j <= radius; j++) {
            if (i*i + j*j <= radius*radius + radius) {      /* 略微扩大以确保边缘完整 */ 
                /* 检查是否在圆弧边缘 */ 
                if ((i+1)*(i+1) + j*j > radius*radius + radius || 
                    i*i + (j+1)*(j+1) > radius*radius + radius) {
                    
                    /* 左上角 */ 
                    lcd_draw_pixel(x + radius - i, y + radius - j, color);
                    /* 右上角 */ 
                    lcd_draw_pixel(x + width - radius + i, y + radius - j, color);
                    /* 左下角 */ 
                    lcd_draw_pixel(x + radius - i, y + height - radius + j, color);
                    /* 右下角 */ 
                    lcd_draw_pixel(x + width - radius + i, y + height - radius + j, color);
                }
            }
        }
    }
}

/* 绘制填充圆角矩形 */ 
void lcd_draw_filled_rounded_rectangle(int x, int y, int width, int height, int radius, color_t color) {
    if (!lcd) return;
    
    /* 
    * 功能：绘制一个填充圆角矩形。
    * 参数：x、y：矩形左上角坐标。width、height：矩形的宽度和高度。radius：圆角半径。color：矩形填充颜色。
    * 逻辑：先确保圆角半径不超过矩形宽度或高度的一半。接着调用 lcd_draw_filled_rectangle 绘制中间的矩形
    *      部分，再通过两层循环和圆的方程判断像素点是否在圆角范围内，若在则调用 lcd_draw_pixel 填充四个角。
    */

    /* 确保半径不超过宽度或高度的一半 */ 
    if (radius > width/2) radius = width/2;
    if (radius > height/2) radius = height/2;
    
    /* 绘制中间矩形部分 */ 
    lcd_draw_filled_rectangle(x, y + radius, width, height - 2*radius, color);
    lcd_draw_filled_rectangle(x + radius, y, width - 2*radius, height, color);
    
    /* 绘制四个角的圆弧 */ 
    for (int i = 0; i < radius; i++) {
        for (int j = 0; j < radius; j++) {
            if (i*i + j*j <= radius*radius) {
                /* 左上角填充 */ 
                lcd_draw_pixel(x + radius - i - 1, y + radius - j - 1, color);
                lcd_draw_pixel(x + radius - i, y + radius - j - 1, color);
                lcd_draw_pixel(x + radius - i - 1, y + radius - j, color);
                
                /* 右上角填充 */ 
                lcd_draw_pixel(x + width - radius + i, y + radius - j - 1, color);
                lcd_draw_pixel(x + width - radius + i - 1, y + radius - j - 1, color);
                lcd_draw_pixel(x + width - radius + i, y + radius - j, color);
                
                /* 左下角填充 */ 
                lcd_draw_pixel(x + radius - i - 1, y + height - radius + j, color);
                lcd_draw_pixel(x + radius - i, y + height - radius + j, color);
                lcd_draw_pixel(x + radius - i - 1, y + height - radius + j - 1, color);
                
                /* 右下角填充 */ 
                lcd_draw_pixel(x + width - radius + i, y + height - radius + j, color);
                lcd_draw_pixel(x + width - radius + i - 1, y + height - radius + j, color);
                lcd_draw_pixel(x + width - radius + i, y + height - radius + j - 1, color);
            }
        }
    }
}

/* 计算文本宽度 */ 
int lcd_get_text_width(const char *text) {
    if (!text || !lcd) return 0;
    
    /* 
    * 计算缩放比例
    * 调用 stbtt_ScaleForPixelHeight 函数，根据当前字体和字体大小计算缩放比例 scale
    */
    float scale = stbtt_ScaleForPixelHeight(&font, font_size);
    float xpos = 0;             /* 累加文本的总宽度 */
    int i = 0;                  /* 字符串的索引 */
    int len = strlen(text);     /* 字符串的长度 */
    
    /* 遍历文本 */
    while (i < len) {
        int codepoint;  /* decode_utf8 函数将 UTF-8 字符解码为 Unicode 码点 codepoint */
        int char_len = decode_utf8(&text[i], &codepoint);   /* 字符的字节长度 */
        
        /*
        * 调用 stbtt_GetCodepointHMetrics 函数获取当前字符的水平度量信息，
        * 包括字符的前进宽度 advance 和左部空白 lsb，将 advance * scale 累加到 xpos。
        */
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);
        xpos += (advance * scale);
        
        /*
        * 若不是最后一个字符，解码下一个字符的码点 next_codepoint，
        * 调用 stbtt_GetCodepointKernAdvance 函数获取当前字符
        * 和下一个字符之间的字距调整值，将其乘以 scale 后累加到 xpos。
        * i 增加 char_len，处理下一个字符。
        */
        if (i + char_len < len) {
            int next_codepoint;
            decode_utf8(&text[i + char_len], &next_codepoint);
            xpos += scale * stbtt_GetCodepointKernAdvance(&font, codepoint, next_codepoint);
        }
        
        i += char_len;
    }
    
    return (int)ceil(xpos);     /* 使用 ceil 函数向上取整 xpos，并将其转换为整数后返回 */
}

/* 计算文本高度 */ 
int lcd_get_text_height(void) {
    if (!lcd) return 0;
    
    /*
    * 计算缩放比例
    * 调用 stbtt_ScaleForPixelHeight 函数，根据当前字体和字体大小计算缩放比例 scale。
    * 包括字符的前进宽度 advance 和左部空白 lsb，将 advance * scale 累加到 xpos。
    */
    float scale = stbtt_ScaleForPixelHeight(&font, font_size);

    /*
    * 获取字体垂直度量信息
    * 调用 stbtt_GetFontVMetrics 函数获取字体的垂直度量信息，包括基线以上的高度 ascent 
    * 和基线以下的深度 descent。
    */
    int ascent, descent;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, 0);

    /*
    * 计算 (ascent - descent) * scale，
    * 将结果转换为整数后返回，该值即为文本的高度。
    */
    return (int)((ascent - descent) * scale);   
}

/* 渲染文字 */
void lcd_render_text(const char *text, int x, int y, color_t text_color, int font_size) {
    if (!text || !lcd) return;
    
    /* 边界检查与初始化 */
    float scale = stbtt_ScaleForPixelHeight(&font, font_size);  /* 根据当前字体和字体大小计算缩放比例 */
    int ascent, baseline;
    stbtt_GetFontVMetrics(&font, &ascent, 0, 0);    /* 获取字体的垂直度量信息，ascent 表示基线以上的高度 */
    baseline = (int)(ascent * scale);   /* 计算基线相对于起始 y 坐标的位置 */

    float xpos = (float)x;      /* 初始化当前字符的 x 坐标 */
    int len = strlen(text);     /* 获取文本字符串的长度 */
    int i = 0;
    
    /* 遍历文本 */
    while (i < len) {
        int codepoint;

        /*
        * 调用 decode_utf8 函数将 UTF-8 字符解码为 Unicode 码点 codepoint，
        * 并获取该字符的字节长度 char_len
        */
        int char_len = decode_utf8(&text[i], &codepoint);
        
        if (codepoint < 32) { /* 跳过 ASCII 码小于 32 的控制字符 */ 
            i += char_len;
            continue;
        }
        
        /* 获取字符度量信息与位图 */
        int advance, lsb, x0, y0, x1, y1;
        float x_shift = xpos - (float)floor(xpos);
        
        /* 获取当前字符的水平度量信息，包括字符的前进宽度 advance 和左部空白 lsb */
        stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &lsb);

        /* stbtt_GetCodepointBitmapBoxSubpixel：获取当前字符位图的边界框坐标 (x0, y0, x1, y1) */
        stbtt_GetCodepointBitmapBoxSubpixel(&font, codepoint, scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);

        int width = x1 - x0;
        int height = y1 - y0;
        unsigned char *bitmap = (unsigned char*)malloc(width * height);     /* 为当前字符的位图分配内存 */
        
        /* 生成字符位图并渲染 */
        if (bitmap) {
            stbtt_MakeCodepointBitmapSubpixel(&font, bitmap, width, height, width, scale, scale, x_shift, 0, codepoint);
            
            for (int j = 0; j < height; ++j) {
                for (int i = 0; i < width; ++i) {
                    int screen_x = (int)xpos + x0 + i;  /* 计算该像素在屏幕上的坐标 (screen_x, screen_y) */
                    int screen_y = baseline + y0 + j + y;
                    /* 检查坐标是否在屏幕范围内 */
                    if (screen_x >= 0 && screen_x < lcd->width && screen_y >= 0 && screen_y < lcd->height) {
                        unsigned char alpha = bitmap[j * width + i];
                        /* 
                        * 获取该像素的透明度 alpha
                        * 若 alpha > 0，进行简单的 alpha 混合，
                        * 将前景色和背景色按透明度混合后更新屏幕缓冲区。
                        */

                        if (alpha > 0) {    
                            uint16_t bg = lcd->mp[screen_y * lcd->width + screen_x];
                            // 修改为使用 text_color
                            uint16_t r = ((bg & 0xF800) * (255 - alpha) + (text_color & 0xF800) * alpha) / 255;
                            uint16_t g = ((bg & 0x07E0) * (255 - alpha) + (text_color & 0x07E0) * alpha) / 255;
                            uint16_t b = ((bg & 0x001F) * (255 - alpha) + (text_color & 0x001F) * alpha) / 255;
                            lcd->mp[screen_y * lcd->width + screen_x] = r | g | b;
                        }
                    }
                }
            }
            free(bitmap);   /* 释放为位图分配的内存 */
        }

        /* 更新 x 坐标并处理下一个字符 */
        xpos += (advance * scale);  /* 更新 x 坐标，加上当前字符的前进宽度 */
        if (i + char_len < len) {   /* 若不是最后一个字符，获取下一个字符的码点，计算并加上字距调整值 */
            int next_codepoint;
            decode_utf8(&text[i + char_len], &next_codepoint);
            xpos += scale * stbtt_GetCodepointKernAdvance(&font, codepoint, next_codepoint);
        }
        
        i += char_len;  /* 处理下一个字符 */
    }
}

/* 渲染文字（带文本框） */
void lcd_render_text_with_box(const char *text, int x, int y, color_t text_color, 
                              color_t box_color, int padding, BoxStyle style, int radius, int font_size,
                              int box_width, int box_height) {
    /* 
    * text：指向要渲染的 UTF-8 编码文本字符串的指针。
    * x、y：文本渲染起始位置的坐标。
    * text_color：文本的颜色。
    * box_color：文本框的填充颜色。
    * padding：文本与文本框边缘的间距。
    * style：文本框的样式，类型为 BoxStyle，可能是矩形或圆角矩形。
    * radius：若 style 为圆角矩形，该参数指定圆角的半径。 
    * font_size：文本的字体大小。
    * box_width：文本框的宽度（长），为 0 时，根据文本量与字体大小调整，文字居中对齐。
    * box_height：文本框的高度（宽），为 0 时，根据文本量与字体大小调整，文字居中对齐。
    */
    if (!text || !lcd) return;
    
    /* 设置字体大小 */ 
    lcd_set_font_size(font_size);

    /* 如果 box_width 或 box_height 为 0，则根据文本内容计算文本框大小 */
    if (box_width == 0 || box_height == 0) {
        box_width = lcd_get_text_width(text) + 2 * padding;
        box_height = lcd_get_text_height() + 2 * padding;
    }

    /* 
    * 绘制文本框
    * 若 style 为 BOX_STYLE_RECTANGLE，调用 lcd_draw_filled_rectangle 函数绘制填充矩形文本框。
    * 矩形左上角坐标为 (x - padding, y - padding)，宽度为 box_width，
    * 高度为 box_height，颜色为 box_color。
    *
    * 若 style 不是 BOX_STYLE_RECTANGLE，调用 lcd_draw_filled_rounded_rectangle 函数绘制
    * 填充圆角矩形文本框，参数含义与矩形类似，额外传入圆角半径 radius。
    */ 
    if (style == BOX_STYLE_RECTANGLE) {
        lcd_draw_filled_rectangle(x - padding, y - padding, box_width, box_height, box_color);
    } else {
        lcd_draw_filled_rounded_rectangle(x - padding, y - padding, box_width, box_height, radius, box_color);
    }
    
    /* 
    * 绘制文字
    * 调用 lcd_render_text 函数在指定位置 (x, y) 以 text_color 颜色渲染文本 text
    */ 
    lcd_render_text(text, x, y, text_color, font_size);
}

