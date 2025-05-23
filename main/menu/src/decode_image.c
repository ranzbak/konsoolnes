/* SPI Master example: jpeg decoder.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
The image used for the effect on the LCD in the SPI master example is stored in flash
as a jpeg file. This file contains the decode_image routine, which uses the tiny JPEG
decoder library in ROM to decode this JPEG into a format that can be sent to the display.

Keep in mind that the decoder library cannot handle progressive files (will give
``Image decoder: jd_prepare failed (8)`` as an error) so make sure to save in the correct
format if you want to use a different image file.
*/

#include "driver/jpeg_decode.h"
#include <decode_image.h>
#include <string.h>

// Reference the binary-included jpeg file -- defined by the build
extern const uint8_t image_jpg_start[] asm("_binary_lib_menu_data_image_jpg_start");
extern const uint8_t image_jpg_end[] asm("_binary_lib_menu_data_image_jpg_end");
// Define the height and width of the jpeg file. Make sure this matches the actual jpeg
// dimensions.
#define IMAGE_W 113 // 336
#define IMAGE_H 256

static const char* TAG = "ImageDec";

// Data that is passed from the decoder function to the infunc/outfunc functions.
typedef struct
{
    const unsigned char* inData;  // Pointer to jpeg data
    int                  inPos;   // Current position in jpeg data
    uint16_t**           outData; // Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
    int                  outW;    // Width of the resulting file
    int                  outH;    // Height of the resulting file
} JpegDev;

// Input function for jpeg decoder. Just returns bytes from the inData field of the JpegDev structure.
//  static UINT infunc(JDEC *decoder, BYTE *buf, UINT len)
//  {
//      //Read bytes from input file
//      JpegDev *jd=(JpegDev*)decoder->device;
//      if (buf!=NULL) memcpy(buf, jd->inData+jd->inPos, len);
//      jd->inPos+=len;
//      return len;
//  }

// //Output function. Re-encodes the RGB888 data from the decoder as big-endian RGB565 and
// //stores it in the outData array of the JpegDev structure.
// static UINT outfunc(JDEC *decoder, void *bitmap, JRECT *rect)
// {
//     JpegDev *jd=(JpegDev*)decoder->device;
//     uint8_t *in=(uint8_t*)bitmap;
//     for (int y=rect->top; y<=rect->bottom; y++) {
//         for (int x=rect->left; x<=rect->right; x++) {
//             //We need to convert the 3 bytes in `in` to a rgb565 value.
//             uint16_t v=0;
//             v|=((in[0]>>3)<<11);
//             v|=((in[1]>>2)<<5);
//             v|=((in[2]>>3)<<0);
//             //The LCD wants the 16-bit value in big-endian, so swap bytes
//             v=(v>>8)|(v<<8);
//             jd->outData[y][x]=v;
//             in+=3;
//         }
//     }
//     return 1;
// }

// Size of the work space for the jpeg decoder.
#define WORKSZ 3100

// Get jpeg decoder handle
// jpeg_decoder_handle_t get_jpgd_handle(void) {
//     static jpeg_decoder_handle_t jpgd_handle = NULL;

//     static jpeg_decode_engine_cfg_t decode_eng_cfg = {
//         .timeout_ms = 40,
//     };

//     if (jpeg_decoder_t == NULL) {
//         ESP_ERROR_CHECK(jpeg_new_decoder_engine(&decode_eng_cfg, &jpgd_handle));

//         jpeg_decode_cfg_t decode_cfg_rgb = {
//             .output_format = JPEG_DECODE_OUT_FORMAT_RGB888,
//             .rgb_order     = JPEG_DEC_RGB_ELEMENT_ORDER_BGR,
//         };

//         jpeg_decode_cfg_t decode_cfg_gray = {
//             .output_format = JPEG_DECODE_OUT_FORMAT_GRAY,
//         };

//         jpeg_decode_memory_alloc_cfg_t rx_mem_cfg = {
//             .buffer_direction = JPEG_DEC_ALLOC_OUTPUT_BUFFER,
//         };

//         jpeg_decode_memory_alloc_cfg_t tx_mem_cfg = {
//             .buffer_direction = JPEG_DEC_ALLOC_INPUT_BUFFER,
//         };
//     }

//     return jpgd_handle;
// }

// Decode the embedded image into pixel lines that can be used with the rest of the logic.
esp_err_t decode_image(uint16_t*** pixels)
{
    // char*   work = NULL;
    // int     r;
    // JDEC    decoder;
    // JpegDev jd;
    // *pixels       = NULL;
    esp_err_t ret = ESP_OK;

    //     //Alocate pixel memory. Each line is an array of IMAGE_W 16-bit pixels; the `*pixels` array itself contains
    //     pointers to these lines. *pixels=calloc(IMAGE_H, sizeof(uint16_t*)); if (*pixels==NULL) {
    //         ESP_LOGE(TAG, "Error allocating memory for lines");
    //         ret=ESP_ERR_NO_MEM;
    //         goto err;
    //     }
    //     for (int i=0; i<IMAGE_H; i++) {
    //         (*pixels)[i]=malloc(IMAGE_W*sizeof(uint16_t));
    //         if ((*pixels)[i]==NULL) {
    //             ESP_LOGE(TAG, "Error allocating memory for line %d", i);
    //             ret=ESP_ERR_NO_MEM;
    //             goto err;
    //         }
    //     }

    //     //Allocate the work space for the jpeg decoder.
    //     work=calloc(WORKSZ, 1);
    //     if (work==NULL) {
    //         ESP_LOGE(TAG, "Cannot allocate workspace");
    //         ret=ESP_ERR_NO_MEM;
    //         goto err;
    //     }

    //     //Populate fields of the JpegDev struct.
    //     jd.inData=image_jpg_start;
    //     jd.inPos=0;
    //     jd.outData=*pixels;
    //     jd.outW=IMAGE_W;
    //     jd.outH=IMAGE_H;

    //     //Prepare and decode the jpeg.
    //     r=jd_prepare(&decoder, infunc, work, WORKSZ, (void*)&jd);
    //     if (r!=JDR_OK) {
    //         ESP_LOGE(TAG, "Image decoder: jd_prepare failed (%d)", r);
    //         ret=ESP_ERR_NOT_SUPPORTED;
    //         goto err;
    //     }
    //     r=jd_decomp(&decoder, outfunc, 0);
    //     if (r!=JDR_OK) {
    //         ESP_LOGE(TAG, "Image decoder: jd_decode failed (%d)", r);
    //         ret=ESP_ERR_NOT_SUPPORTED;
    //         goto err;
    //     }

    //     //All done! Free the work area (as we don't need it anymore) and return victoriously.
    //     //free(work);
    //     return ret;
    // err:
    //     //Something went wrong! Exit cleanly, de-allocating everything we allocated.
    //     if (*pixels!=NULL) {
    //         for (int i=0; i<IMAGE_H; i++) {
    //             free((*pixels)[i]);
    //         }
    //         free(*pixels);
    //     }
    //     free(work);
    return ret;
}
