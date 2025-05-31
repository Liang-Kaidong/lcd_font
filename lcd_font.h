/**********************************************************************************************************
File name:lcd_font.h
Author:Liang Kaidong
Version:V_1.0
Build date: 2025-05-21
Description:This header file is designed to declare functions and global variables pertaining to text box
            rendering, color customization, and text output display. It facilitates modular development and
            offers standard library interfaces. Users must adhere strictly to the development documentation
            to prevent unforeseen complications. Given the code's current implementation constraints, your
            understanding and acceptance of its limitations are appreciated.
Others:Usage requires preservation of original author attribution.
Log:1.Optimize code comments and remove some useless code.
**********************************************************************************************************/

/*
* 头文件保护
* 这部分代码使用预处理器指令防止头文件被重复包含，
* 避免因重复包含导致的编译错误。
*/
#ifndef LCD_FONT_H
#define LCD_FONT_H

#include <stdint.h> 

/* 定义 BoxStyle 枚举类型 */
typedef enum {
    BOX_STYLE_RECTANGLE,    /* 矩形样式 */ 
    BOX_STYLE_ROUNDED       /* 圆角矩形样式 */ 
} BoxStyle;

/* 
* 颜色定义
* 预定义常见颜色的 RGB565 值，可自行添加或在其他文件中定义。
* 注意：RGB565 是一种 16 位颜色格式，每个颜色由 5 位红色、6 位绿色和 5 位蓝色组成。
*      因此，颜色值的范围为 0x0000 到 0xFFFF。
*/
typedef uint16_t color_t;
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_PURPLE  0x8010    
#define COLOR_ORANGE  0xFC00    
#define COLOR_DARKGREEN 0x03E0  
#define COLOR_DARKBLUE  0x0010  
#define COLOR_LIGHTGRAY 0xC618  

/* RGB565 颜色转换 */
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

/* 
* 初始化 LCD 显示屏和字体系统 
* 接收 LCD 设备文件路径和字体文件路径作为参数，成功返回 0，失败返回非 0 值。
*/
int lcd_init(const char *lcd_path, const char *font_path);
void lcd_cleanup(void);     /* 清理 LCD 显示屏和字体系统占用的资源 */

/* 屏幕操作 */
void lcd_clear(color_t color);      /* 清空屏幕为指定颜色 */
void lcd_set_font_size(int size);   /* 设置字体大小 */

/* 
* 文本渲染
* lcd_render_text：渲染普通文本。text 是要渲染的文本内容，x 和 y 是文本的起始坐标，
                   text_color 是文本颜色，font_size 是字体大小。
* lcd_render_text_with_box：带文本框的文本渲染。text 是要渲染的文本内容，x 和 y 是文本的起始坐标，
                            text_color 是文本颜色，box_color 是文本框背景颜色，padding 是文本框内边距，
                            style 是文本框样式（矩形或圆角矩形），radius 是圆角半径，font_size 是字体大小，
                            box_width 和 box_height 是文本框的宽度和高度（ 0，0 时文本框根据文字量大小和文字大小确定）。
*/
/* 普通文本渲染 */
void lcd_render_text( const char *text,     /* 文本内容 */
                      int x, int y,         /* 文本起始坐标 */
                      color_t text_color,   /* 文本颜色 */
                      int font_size         /* 字体大小 */
                    );      
void lcd_render_text_with_box( const char *text,       /* 文本内容 */
                               int x, int y,           /* 文本起始坐标 */
                               color_t text_color,     /* 文本颜色 */
                               color_t box_color,      /* 文本框背景颜色 */
                               int padding,            /* 文本框内边距 */
                               BoxStyle style,         /* 文本框样式 */
                               int radius,             /* 圆角半径 */
                               int font_size,          /* 字体大小 */
                               int box_width,          /* 文本框宽度 */
                               int box_height          /* 文本框高度 */
                             );

/* 获取文本尺寸 */
int lcd_get_text_width(const char *text);       /* 获取文本宽度，text 是要计算宽度的文本字符串 */
int lcd_get_text_height(void);                  /* 当前字体的文本高度 */

/* 
* 图形绘制函数
* lcd_draw_rectangle：绘制矩形边框。x, y 是矩形左上角的坐标，width 和 height 是矩形的宽度和高度，
                      color_t 是边框颜色。
* lcd_draw_filled_rectangle：绘制填充矩形。x, y 是矩形左上角的坐标，width 和 height 是矩形的宽度和高度，
                             color_t 是填充颜色。
* lcd_draw_rounded_rectangle：绘制圆角矩形边框。 x, y 是矩形左上角的坐标，width 和 height 是矩形的宽度和高度，
                              radius 是圆角的半径，color_t 是边框颜色。
* lcd_draw_filled_rounded_rectangle：绘制填充圆角矩形。 x, y 是矩形左上角的坐标，width 和 height 是矩形的宽度和高度，
                                     radius 是圆角的半径，color_t 是填充颜色。
*/
void lcd_draw_rectangle(int x, int y,   /* 文本起始坐标 */
                        int width,      /* 文本宽度 */
                        int height,     /* 文本高度 */
                        color_t color   /* 文本颜色 */
                      );
void lcd_draw_filled_rectangle(int x, int y,     /* 文本起始坐标 */
                               int width,        /* 文本宽度 */
                               int height,       /* 文本高度 */
                               color_t color     /* 文本颜色 */
                              );
void lcd_draw_rounded_rectangle(int x, int y,     /* 文本起始坐标 */
                                int width,        /* 文本宽度 */
                                int height,       /* 文本高度 */
                                int radius,       /* 文本圆角半径 */
                                color_t color     /* 文本颜色 */
                              );
void lcd_draw_filled_rounded_rectangle(int x, int y,     /* 文本起始坐标 */
                                       int width,        /* 文本宽度 */
                                       int height,       /* 文本高度 */
                                       int radius,       /* 文本圆角半径 */
                                       color_t color     /* 文本颜色 */
                                      );

/* 结束头文件保护 */
#endif /* LCD_FONT_H */ 