/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** bitmap.c
**
** Bitmap object manipulation routines
** $Id: bitmap.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include "esp_heap_caps.h"
#include <bitmap.h>
#include <noftypes.h>
#include <stdio.h>
#include <string.h>

void bmp_clear(const bitmap_t* bitmap, uint8_t color)
{
    memset(bitmap->data, color, bitmap->pitch * bitmap->height);
}

static bitmap_t* _make_bitmap(uint8_t* data_addr, bool hw, int width,
                              int height, int pitch, int overdraw)
{
    bitmap_t* bitmap;
    int       i;

    /* quick safety check */
    if (NULL == data_addr)
        return NULL;

    /* Make sure to add in space for line pointers */
    // Pretty dirty hack to allocate the bitmap + line pointers in one go
    bitmap = malloc(sizeof(bitmap_t) + (sizeof(uint8_t*) * height));
    if (NULL == bitmap)
        return NULL;

    bitmap->hardware = hw;
    bitmap->height   = height;
    bitmap->width    = width;
    bitmap->data     = data_addr;
    bitmap->pitch    = pitch + (overdraw * 2);

    /* Set up line pointers */
    bitmap->line[0] = bitmap->data + overdraw;

    for (i = 1; i < height; i++)
        bitmap->line[i] = bitmap->line[i - 1] + bitmap->pitch;

    return bitmap;
}

/* Allocate and initialize a bitmap structure */
bitmap_t* bmp_create(int width, int height, int overdraw)
{
    uint8_t* addr;
    int      pitch;

    pitch = width + (overdraw * 2);                                                                       /* left and right */
    addr  = heap_caps_malloc((pitch * (height + 20)) + overdraw + 3, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM); /* add max 32-bit aligned adjustment */
    if (NULL == addr)
        return NULL;

    return _make_bitmap(addr, true, width, height, width, overdraw);
}

/* allocate and initialize a hardware bitmap */
bitmap_t* bmp_createhw(uint8_t* addr, int width, int height, int pitch)
{
    return _make_bitmap(addr, true, width, height, pitch, 0); /* zero overdraw */
}

/* Deallocate space for a bitmap structure */
void bmp_destroy(bitmap_t** bitmap)
{
    if (*bitmap)
    {
        if ((*bitmap)->data && false == (*bitmap)->hardware)
            free((*bitmap)->data);
        free(*bitmap);
        *bitmap = NULL;
    }
}

/*
** $Log: bitmap.c,v $
** Revision 1.2  2001/04/27 14:37:11  neil
** wheeee
**
** Revision 1.1.1.1  2001/04/27 07:03:54  neil
** initial
**
** Revision 1.16  2000/11/05 16:37:18  matt
** rolled rgb.h into bitmap.h
**
** Revision 1.15  2000/10/10 13:58:13  matt
** stroustrup squeezing his way in the door
**
** Revision 1.14  2000/09/18 02:06:48  matt
** -pedantic is your friend
**
** Revision 1.13  2000/08/13 13:16:30  matt
** bugfix for alignment adjustment
**
** Revision 1.12  2000/07/24 04:31:43  matt
** pitch/data area on non-hw bitmaps get padded to 32-bit boundaries
**
** Revision 1.11  2000/07/17 01:52:27  matt
** made sure last line of all source files is a newline
**
** Revision 1.10  2000/07/09 14:43:01  matt
** pitch is now configurable for bmp_createhw()
**
** Revision 1.9  2000/07/06 17:55:57  matt
** two big bugs fixed
**
** Revision 1.8  2000/07/06 17:38:11  matt
** replaced missing string.h include
**
** Revision 1.7  2000/07/06 16:46:57  matt
** added bmp_clear() routine
**
** Revision 1.6  2000/06/26 04:56:24  matt
** minor cleanup
**
** Revision 1.5  2000/06/09 15:12:25  matt
** initial revision
**
*/
