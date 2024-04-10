/* ===========================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         https://github.com/rougier/freetype-gl
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 */
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "ftt-texture-font.h"
#include "ftt-platform.h"
#include "ftt-utf8-utils.h"

#define HRES  64
#define HRESf 64.f
#define DPI   72

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

#ifdef _VISUAL_STUDIO
#pragma warning(disable: 4267)
#endif

const char* FTToolsDefaultChars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f\xc2\x80\xc2\x81\xc2\x82\xc2\x83\xc2\x84\xc2\x85\xc2\x86\xc2\x87\xc2\x88\xc2\x89\xc2\x8a\xc2\x8b\xc2\x8c\xc2\x8d\xc2\x8e\xc2\x8f\xc2\x90\xc2\x91\xc2\x92\xc2\x93\xc2\x94\xc2\x95\xc2\x96\xc2\x97\xc2\x98\xc2\x99\xc2\x9a\xc2\x9b\xc2\x9c\xc2\x9d\xc2\x9e\xc2\x9f\xc2\xa0\xc2\xa1\xc2\xa2\xc2\xa3\xc2\xa4\xc2\xa5\xc2\xa6\xc2\xa7\xc2\xa8\xc2\xa9\xc2\xaa\xc2\xab\xc2\xac\xc2\xad\xc2\xae\xc2\xaf\xc2\xb0\xc2\xb1\xc2\xb2\xc2\xb3\xc2\xb4\xc2\xb5\xc2\xb6\xc2\xb7\xc2\xb8\xc2\xb9\xc2\xba\xc2\xbb\xc2\xbc\xc2\xbd\xc2\xbe\xc2\xbf\xc3\x80\xc3\x81\xc3\x82\xc3\x83\xc3\x84\xc3\x85\xc3\x86\xc3\x87\xc3\x88\xc3\x89\xc3\x8a\xc3\x8b\xc3\x8c\xc3\x8d\xc3\x8e\xc3\x8f\xc3\x90\xc3\x91\xc3\x92\xc3\x93\xc3\x94\xc3\x95\xc3\x96\xc3\x97\xc3\x98\xc3\x99\xc3\x9a\xc3\x9b\xc3\x9c\xc3\x9d\xc3\x9e\xc3\x9f\xc3\xa0\xc3\xa1\xc3\xa2\xc3\xa3\xc3\xa4\xc3\xa5\xc3\xa6\xc3\xa7\xc3\xa8\xc3\xa9\xc3\xaa\xc3\xab\xc3\xac\xc3\xad\xc3\xae\xc3\xaf\xc3\xb0\xc3\xb1\xc3\xb2\xc3\xb3\xc3\xb4\xc3\xb5\xc3\xb6\xc3\xb7\xc3\xb8\xc3\xb9\xc3\xba\xc3\xbb\xc3\xbc\xc3\xbd\xc3\xbe\xc3\xbf\xc4\x80\xc4\x81\xc4\x82\xc4\x83\xc4\x84\xc4\x85\xc4\x86\xc4\x87\xc4\x88\xc4\x89\xc4\x8a\xc4\x8b\xc4\x8c\xc4\x8d\xc4\x8e\xc4\x8f\xc4\x90\xc4\x91\xc4\x92\xc4\x93\xc4\x94\xc4\x95\xc4\x96\xc4\x97\xc4\x98\xc4\x99\xc4\x9a\xc4\x9b\xc4\x9c\xc4\x9d\xc4\x9e\xc4\x9f\xc4\xa0\xc4\xa1\xc4\xa2\xc4\xa3\xc4\xa4\xc4\xa5\xc4\xa6\xc4\xa7\xc4\xa8\xc4\xa9\xc4\xaa\xc4\xab\xc4\xac\xc4\xad\xc4\xae\xc4\xaf\xc4\xb0\xc4\xb1\xc4\xb2\xc4\xb3\xc4\xb4\xc4\xb5\xc4\xb6\xc4\xb7\xc4\xb8\xc4\xb9\xc4\xba\xc4\xbb\xc4\xbc\xc4\xbd\xc4\xbe\xc4\xbf\xc5\x80\xc5\x81\xc5\x82\xc5\x83\xc5\x84\xc5\x85\xc5\x86\xc5\x87\xc5\x88\xc5\x89\xc5\x8a\xc5\x8b\xc5\x8c\xc5\x8d\xc5\x8e\xc5\x8f\xc5\x90\xc5\x91\xc5\x92\xc5\x93\xc5\x94\xc5\x95\xc5\x96\xc5\x97\xc5\x98\xc5\x99\xc5\x9a\xc5\x9b\xc5\x9c\xc5\x9d\xc5\x9e\xc5\x9f\xc5\xa0\xc5\xa1\xc5\xa2\xc5\xa3\xc5\xa4\xc5\xa5\xc5\xa6\xc5\xa7\xc5\xa8\xc5\xa9\xc5\xaa\xc5\xab\xc5\xac\xc5\xad\xc5\xae\xc5\xaf\xc5\xb0\xc5\xb1\xc5\xb2\xc5\xb3\xc5\xb4\xc5\xb5\xc5\xb6\xc5\xb7\xc5\xb8\xc5\xb9\xc5\xba\xc5\xbb\xc5\xbc\xc5\xbd\xc5\xbe\xc5\xbf\xc6\x80\xc6\x81\xc6\x82\xc6\x83\xc6\x84\xc6\x85\xc6\x86\xc6\x87\xc6\x88\xc6\x89\xc6\x8a\xc6\x8b\xc6\x8c\xc6\x8d\xc6\x8e\xc6\x8f\xc6\x90\xc6\x91\xc6\x92\xc6\x93\xc6\x94\xc6\x95\xc6\x96\xc6\x97\xc6\x98\xc6\x99\xc6\x9a\xc6\x9b\xc6\x9c\xc6\x9d\xc6\x9e\xc6\x9f\xc6\xa0\xc6\xa1\xc6\xa2\xc6\xa3\xc6\xa4\xc6\xa5\xc6\xa6\xc6\xa7\xc6\xa8\xc6\xa9\xc6\xaa\xc6\xab\xc6\xac\xc6\xad\xc6\xae\xc6\xaf\xc6\xb0\xc6\xb1\xc6\xb2\xc6\xb3\xc6\xb4\xc6\xb5\xc6\xb6\xc6\xb7\xc6\xb8\xc6\xb9\xc6\xba\xc6\xbb\xc6\xbc\xc6\xbd\xc6\xbe\xc6\xbf\xc7\x80\xc7\x81\xc7\x82\xc7\x83\xc7\x84\xc7\x85\xc7\x86\xc7\x87\xc7\x88\xc7\x89\xc7\x8a\xc7\x8b\xc7\x8c\xc7\x8d\xc7\x8e\xc7\x8f\xc7\x90\xc7\x91\xc7\x92\xc7\x93\xc7\x94\xc7\x95\xc7\x96\xc7\x97\xc7\x98\xc7\x99\xc7\x9a\xc7\x9b\xc7\x9c\xc7\x9d\xc7\x9e\xc7\x9f\xc7\xa0\xc7\xa1\xc7\xa2\xc7\xa3\xc7\xa4\xc7\xa5\xc7\xa6\xc7\xa7\xc7\xa8\xc7\xa9\xc7\xaa\xc7\xab\xc7\xac\xc7\xad\xc7\xae\xc7\xaf\xc7\xb0\xc7\xb1\xc7\xb2\xc7\xb3\xc7\xb4\xc7\xb5\xc7\xb6\xc7\xb7\xc7\xb8\xc7\xb9\xc7\xba\xc7\xbb\xc7\xbc\xc7\xbd\xc7\xbe\xc7\xbf\xc8\x80\xc8\x81\xc8\x82\xc8\x83\xc8\x84\xc8\x85\xc8\x86\xc8\x87\xc8\x88\xc8\x89\xc8\x8a\xc8\x8b\xc8\x8c\xc8\x8d\xc8\x8e\xc8\x8f\xc8\x90\xc8\x91\xc8\x92\xc8\x93\xc8\x94\xc8\x95\xc8\x96\xc8\x97\xc8\x98\xc8\x99\xc8\x9a\xc8\x9b\xc8\x9c\xc8\x9d\xc8\x9e\xc8\x9f\xc8\xa0\xc8\xa1\xc8\xa2\xc8\xa3\xc8\xa4\xc8\xa5\xc8\xa6\xc8\xa7\xc8\xa8\xc8\xa9\xc8\xaa\xc8\xab\xc8\xac\xc8\xad\xc8\xae\xc8\xaf\xc8\xb0\xc8\xb1\xc8\xb2\xc8\xb3\xc8\xb4\xc8\xb5\xc8\xb6\xc8\xb7\xc8\xb8\xc8\xb9\xc8\xba\xc8\xbb\xc8\xbc\xc8\xbd\xc8\xbe\xc8\xbf\xc9\x80\xc9\x81\xc9\x82\xc9\x83\xc9\x84\xc9\x85\xc9\x86\xc9\x87\xc9\x88\xc9\x89\xc9\x8a\xc9\x8b\xc9\x8c\xc9\x8d\xc9\x8e\xc9\x8f\xc9\x90\xc9\x91\xc9\x92\xc9\x93\xc9\x94\xc9\x95\xc9\x96\xc9\x97\xc9\x98\xc9\x99\xc9\x9a\xc9\x9b\xc9\x9c\xc9\x9d\xc9\x9e\xc9\x9f\xc9\xa0\xc9\xa1\xc9\xa2\xc9\xa3\xc9\xa4\xc9\xa5\xc9\xa6\xc9\xa7\xc9\xa8\xc9\xa9\xc9\xaa\xc9\xab\xc9\xac\xc9\xad\xc9\xae\xc9\xaf\xc9\xb0\xc9\xb1\xc9\xb2\xc9\xb3\xc9\xb4\xc9\xb5\xc9\xb6\xc9\xb7\xc9\xb8\xc9\xb9\xc9\xba\xc9\xbb\xc9\xbc\xc9\xbd\xc9\xbe\xc9\xbf\xca\x80\xca\x81\xca\x82\xca\x83\xca\x84\xca\x85\xca\x86\xca\x87\xca\x88\xca\x89\xca\x8a\xca\x8b\xca\x8c\xca\x8d\xca\x8e\xca\x8f\xca\x90\xca\x91\xca\x92\xca\x93\xca\x94\xca\x95\xca\x96\xca\x97\xca\x98\xca\x99\xca\x9a\xca\x9b\xca\x9c\xca\x9d\xca\x9e\xca\x9f\xca\xa0\xca\xa1\xca\xa2\xca\xa3\xca\xa4\xca\xa5\xca\xa6\xca\xa7\xca\xa8\xca\xa9\xca\xaa\xca\xab\xca\xac\xca\xad\xca\xae\xca\xaf\xca\xb0\xca\xb1\xca\xb2\xca\xb3\xca\xb4\xca\xb5\xca\xb6\xca\xb7\xca\xb8\xca\xb9\xca\xba\xca\xbb\xca\xbc\xca\xbd\xca\xbe\xca\xbf\xcb\x80\xcb\x81\xcb\x82\xcb\x83\xcb\x84\xcb\x85\xcb\x86\xcb\x87\xcb\x88\xcb\x89\xcb\x8a\xcb\x8b\xcb\x8c\xcb\x8d\xcb\x8e\xcb\x8f\xcb\x90\xcb\x91\xcb\x92\xcb\x93\xcb\x94\xcb\x95\xcb\x96\xcb\x97\xcb\x98\xcb\x99\xcb\x9a\xcb\x9b\xcb\x9c\xcb\x9d\xcb\x9e\xcb\x9f\xcb\xa0\xcb\xa1\xcb\xa2\xcb\xa3\xcb\xa4\xcb\xa5\xcb\xa6\xcb\xa7\xcb\xa8\xcb\xa9\xcb\xaa\xcb\xab\xcb\xac\xcb\xad\xcb\xae\xcb\xaf\xcb\xb0\xcb\xb1\xcb\xb2\xcb\xb3\xcb\xb4\xcb\xb5\xcb\xb6\xcb\xb7\xcb\xb8\xcb\xb9\xcb\xba\xcb\xbb\xcb\xbc\xcb\xbd\xcb\xbe\xcb\xbf\xcc\x80\xcc\x81\xcc\x82\xcc\x83\xcc\x84\xcc\x85\xcc\x86\xcc\x87\xcc\x88\xcc\x89\xcc\x8a\xcc\x8b\xcc\x8c\xcc\x8d\xcc\x8e\xcc\x8f\xcc\x90\xcc\x91\xcc\x92\xcc\x93\xcc\x94\xcc\x95\xcc\x96\xcc\x97\xcc\x98\xcc\x99\xcc\x9a\xcc\x9b\xcc\x9c\xcc\x9d\xcc\x9e\xcc\x9f\xcc\xa0\xcc\xa1\xcc\xa2\xcc\xa3\xcc\xa4\xcc\xa5\xcc\xa6\xcc\xa7\xcc\xa8\xcc\xa9\xcc\xaa\xcc\xab\xcc\xac\xcc\xad\xcc\xae\xcc\xaf\xcc\xb0\xcc\xb1\xcc\xb2\xcc\xb3\xcc\xb4\xcc\xb5\xcc\xb6\xcc\xb7\xcc\xb8\xcc\xb9\xcc\xba\xcc\xbb\xcc\xbc\xcc\xbd\xcc\xbe\xcc\xbf\xcd\x80\xcd\x81\xcd\x82\xcd\x83\xcd\x84\xcd\x85\xcd\x86\xcd\x87\xcd\x88\xcd\x89\xcd\x8a\xcd\x8b\xcd\x8c\xcd\x8d\xcd\x8e\xcd\x8f\xcd\x90\xcd\x91\xcd\x92\xcd\x93\xcd\x94\xcd\x95\xcd\x96\xcd\x97\xcd\x98\xcd\x99\xcd\x9a\xcd\x9b\xcd\x9c\xcd\x9d\xcd\x9e\xcd\x9f\xcd\xa0\xcd\xa1\xcd\xa2\xcd\xa3\xcd\xa4\xcd\xa5\xcd\xa6\xcd\xa7\xcd\xa8\xcd\xa9\xcd\xaa\xcd\xab\xcd\xac\xcd\xad\xcd\xae\xcd\xaf\xcd\xb0\xcd\xb1\xcd\xb2\xcd\xb3\xcd\xb4\xcd\xb5\xcd\xb6\xcd\xb7\xcd\xb8\xcd\xb9\xcd\xba\xcd\xbb\xcd\xbc\xcd\xbd\xcd\xbe\xcd\xbf\xce\x80\xce\x81\xce\x82\xce\x83\xce\x84\xce\x85\xce\x86\xce\x87\xce\x88\xce\x89\xce\x8a\xce\x8b\xce\x8c\xce\x8d\xce\x8e\xce\x8f\xce\x90\xce\x91\xce\x92\xce\x93\xce\x94\xce\x95\xce\x96\xce\x97\xce\x98\xce\x99\xce\x9a\xce\x9b\xce\x9c\xce\x9d\xce\x9e\xce\x9f\xce\xa0\xce\xa1\xce\xa2\xce\xa3\xce\xa4\xce\xa5\xce\xa6\xce\xa7\xce\xa8\xce\xa9\xce\xaa\xce\xab\xce\xac\xce\xad\xce\xae\xce\xaf\xce\xb0\xce\xb1\xce\xb2\xce\xb3\xce\xb4\xce\xb5\xce\xb6\xce\xb7\xce\xb8\xce\xb9\xce\xba\xce\xbb\xce\xbc\xce\xbd\xce\xbe\xce\xbf\xcf\x80\xcf\x81\xcf\x82\xcf\x83\xcf\x84\xcf\x85\xcf\x86\xcf\x87\xcf\x88\xcf\x89\xcf\x8a\xcf\x8b\xcf\x8c\xcf\x8d\xcf\x8e\xcf\x8f\xcf\x90\xcf\x91\xcf\x92\xcf\x93\xcf\x94\xcf\x95\xcf\x96\xcf\x97\xcf\x98\xcf\x99\xcf\x9a\xcf\x9b\xcf\x9c\xcf\x9d\xcf\x9e\xcf\x9f\xcf\xa0\xcf\xa1\xcf\xa2\xcf\xa3\xcf\xa4\xcf\xa5\xcf\xa6\xcf\xa7\xcf\xa8\xcf\xa9\xcf\xaa\xcf\xab\xcf\xac\xcf\xad\xcf\xae\xcf\xaf\xcf\xb0\xcf\xb1\xcf\xb2\xcf\xb3\xcf\xb4\xcf\xb5\xcf\xb6\xcf\xb7\xcf\xb8\xcf\xb9\xcf\xba\xcf\xbb\xcf\xbc\xcf\xbd\xcf\xbe\xcf\xbf\xd0\x80\xd0\x81\xd0\x82\xd0\x83\xd0\x84\xd0\x85\xd0\x86\xd0\x87\xd0\x88\xd0\x89\xd0\x8a\xd0\x8b\xd0\x8c\xd0\x8d\xd0\x8e\xd0\x8f\xd0\x90\xd0\x91\xd0\x92\xd0\x93\xd0\x94\xd0\x95\xd0\x96\xd0\x97\xd0\x98\xd0\x99\xd0\x9a\xd0\x9b\xd0\x9c\xd0\x9d\xd0\x9e\xd0\x9f\xd0\xa0\xd0\xa1\xd0\xa2\xd0\xa3\xd0\xa4\xd0\xa5\xd0\xa6\xd0\xa7\xd0\xa8\xd0\xa9\xd0\xaa\xd0\xab\xd0\xac\xd0\xad\xd0\xae\xd0\xaf\xd0\xb0\xd0\xb1\xd0\xb2\xd0\xb3\xd0\xb4\xd0\xb5\xd0\xb6\xd0\xb7\xd0\xb8\xd0\xb9\xd0\xba\xd0\xbb\xd0\xbc\xd0\xbd\xd0\xbe\xd0\xbf\xd1\x80\xd1\x81\xd1\x82\xd1\x83\xd1\x84\xd1\x85\xd1\x86\xd1\x87\xd1\x88\xd1\x89\xd1\x8a\xd1\x8b\xd1\x8c\xd1\x8d\xd1\x8e\xd1\x8f\xd1\x90\xd1\x91\xd1\x92\xd1\x93\xd1\x94\xd1\x95\xd1\x96\xd1\x97\xd1\x98\xd1\x99\xd1\x9a\xd1\x9b\xd1\x9c\xd1\x9d\xd1\x9e\xd1\x9f\xd1\xa0\xd1\xa1\xd1\xa2\xd1\xa3\xd1\xa4\xd1\xa5\xd1\xa6\xd1\xa7\xd1\xa8\xd1\xa9\xd1\xaa\xd1\xab\xd1\xac\xd1\xad\xd1\xae\xd1\xaf\xd1\xb0\xd1\xb1\xd1\xb2\xd1\xb3\xd1\xb4\xd1\xb5\xd1\xb6\xd1\xb7\xd1\xb8\xd1\xb9\xd1\xba\xd1\xbb\xd1\xbc\xd1\xbd\xd1\xbe\xd1\xbf\xd2\x80\xd2\x81\xd2\x82\xd2\x83\xd2\x84\xd2\x85\xd2\x86\xd2\x87\xd2\x88\xd2\x89\xd2\x8a\xd2\x8b\xd2\x8c\xd2\x8d\xd2\x8e\xd2\x8f\xd2\x90\xd2\x91\xd2\x92\xd2\x93\xd2\x94\xd2\x95\xd2\x96\xd2\x97\xd2\x98\xd2\x99\xd2\x9a\xd2\x9b\xd2\x9c\xd2\x9d\xd2\x9e\xd2\x9f\xd2\xa0\xd2\xa1\xd2\xa2\xd2\xa3\xd2\xa4\xd2\xa5\xd2\xa6\xd2\xa7\xd2\xa8\xd2\xa9\xd2\xaa\xd2\xab\xd2\xac\xd2\xad\xd2\xae\xd2\xaf\xd2\xb0\xd2\xb1\xd2\xb2\xd2\xb3\xd2\xb4\xd2\xb5\xd2\xb6\xd2\xb7\xd2\xb8\xd2\xb9\xd2\xba\xd2\xbb\xd2\xbc\xd2\xbd\xd2\xbe\xd2\xbf\xd3\x80\xd3\x81\xd3\x82\xd3\x83\xd3\x84\xd3\x85\xd3\x86\xd3\x87\xd3\x88\xd3\x89\xd3\x8a\xd3\x8b\xd3\x8c\xd3\x8d\xd3\x8e\xd3\x8f\xd3\x90\xd3\x91\xd3\x92\xd3\x93\xd3\x94\xd3\x95\xd3\x96\xd3\x97\xd3\x98\xd3\x99\xd3\x9a\xd3\x9b\xd3\x9c\xd3\x9d\xd3\x9e\xd3\x9f\xd3\xa0\xd3\xa1\xd3\xa2\xd3\xa3\xd3\xa4\xd3\xa5\xd3\xa6\xd3\xa7\xd3\xa8\xd3\xa9\xd3\xaa\xd3\xab\xd3\xac\xd3\xad\xd3\xae\xd3\xaf\xd3\xb0\xd3\xb1\xd3\xb2\xd3\xb3\xd3\xb4\xd3\xb5\xd3\xb6\xd3\xb7\xd3\xb8\xd3\xb9\xd3\xba\xd3\xbb\xd3\xbc\xd3\xbd\xd3\xbe\xd3\xbf\xd4\x80\xd4\x81\xd4\x82\xd4\x83\xd4\x84\xd4\x85\xd4\x86\xd4\x87\xd4\x88\xd4\x89\xd4\x8a\xd4\x8b\xd4\x8c\xd4\x8d\xd4\x8e\xd4\x8f\xd4\x90\xd4\x91\xd4\x92\xd4\x93\xd4\x94\xd4\x95\xd4\x96\xd4\x97\xd4\x98\xd4\x99\xd4\x9a\xd4\x9b\xd4\x9c\xd4\x9d\xd4\x9e\xd4\x9f\xd4\xa0\xd4\xa1\xd4\xa2\xd4\xa3\xd4\xa4\xd4\xa5\xd4\xa6\xd4\xa7\xd4\xa8\xd4\xa9\xd4\xaa\xd4\xab\xd4\xac\xd4\xad\xd4\xae\xd4\xaf\xe2\x80\x93\xe2\x80\x94\xe2\x80\x95\xe2\x80\x98\xe2\x80\x99\xe2\x80\x9a\xe2\x80\x9b\xe2\x80\x9c\xe2\x80\x9d\xe2\x80\x9e";

// ------------------------------------------------- texture_font_load_face ---
static int
texture_font_load_face(texture_font_t *self, float size,
        FT_Library *library, FT_Face *face)
{
    FT_Error error;
#if 0
    FT_Matrix matrix = {
        (int)((1.0/HRES) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};
#endif

    assert(library);
    assert(size);

    /* Initialize library */
    error = FT_Init_FreeType(library);
    if(error) {
        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                FT_Errors[error].code, FT_Errors[error].message);
        return 0;
    }

    /* Load face */
    switch (self->location) {
    case TEXTURE_FONT_FILE:
        error = FT_New_Face(*library, self->filename, 0, face);
        break;

    case TEXTURE_FONT_MEMORY:
        error = FT_New_Memory_Face(*library,
            self->memory.base, self->memory.size, 0, face);
        break;
    }

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Select charmap */
    error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Set char size */
#if 0
    error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);
#endif
    error = FT_Set_Pixel_Sizes(*face, 0, (int)(size));

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        return 0;
    }

#if 0
    /* Set transform matrix */
    FT_Set_Transform(*face, &matrix, NULL);
#endif

    return 1;
}

static int
texture_font_get_face_with_size(texture_font_t *self, float size,
        FT_Library *library, FT_Face *face)
{
    return texture_font_load_face(self, size, library, face);
}

static int
texture_font_get_face(texture_font_t *self,
        FT_Library *library, FT_Face *face)
{
    return texture_font_get_face_with_size(self, self->size, library, face);
}

static int
texture_font_get_hires_face(texture_font_t *self,
        FT_Library *library, FT_Face *face)
{
#if 0
    return texture_font_get_face_with_size(self,
            self->size * 100.f, library, face);
#endif
    return texture_font_get_face_with_size(self,
            self->size, library, face);
}

// ------------------------------------------------------ texture_glyph_new ---
texture_glyph_t *
texture_glyph_new(void)
{
    texture_glyph_t *self = (texture_glyph_t *) malloc( sizeof(texture_glyph_t) );
    if(self == NULL) {
        fprintf( stderr,
                "line %d: No more memory for allocating data\n", __LINE__);
        return NULL;
    }

    self->charcode  = -1;
    self->id        = 0;
    self->width     = 0;
    self->height    = 0;
    self->outline_type = 0;
    self->outline_thickness = 0.0;
    self->offset_x  = 0;
    self->offset_y  = 0;
    self->advance_x = 0.0;
    self->advance_y = 0.0;
    self->s0        = 0.0;
    self->t0        = 0.0;
    self->s1        = 0.0;
    self->t1        = 0.0;
    self->kerning   = vector_new( sizeof(kerning_t) );
    return self;
}


// --------------------------------------------------- texture_glyph_delete ---
void
texture_glyph_delete( texture_glyph_t *self )
{
    assert( self );
    vector_delete( self->kerning );
    free( self );
}

// ---------------------------------------------- texture_glyph_get_kerning ---
float
texture_glyph_get_kerning( const texture_glyph_t * self,
                           const char * charcode )
{
    size_t i;
    uint32_t ucharcode = utf8_to_utf32( charcode );

    assert( self );
    for( i=0; i<vector_size(self->kerning); ++i )
    {
        kerning_t * kerning = (kerning_t *) vector_get( self->kerning, i );
        if( kerning->charcode == ucharcode )
        {
            return kerning->kerning;
        }
    }
    return 0;
}


// ------------------------------------------ texture_font_generate_kerning ---
void
texture_font_generate_kerning( texture_font_t *self )
{
    size_t i, j;
    FT_Library library;
    FT_Face face;
    FT_UInt glyph_index, prev_index;
    texture_glyph_t *glyph, *prev_glyph;
    FT_Vector kerning;

    assert( self );

    /* Load font */
    if(!texture_font_get_face(self, &library, &face))
        return;

    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for( i=1; i<self->glyphs->size; ++i )
    {
        glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
        glyph_index = FT_Get_Char_Index( face, glyph->charcode );
        vector_clear( glyph->kerning );

        for( j=1; j<self->glyphs->size; ++j )
        {
            prev_glyph = *(texture_glyph_t **) vector_get( self->glyphs, j );
            prev_index = FT_Get_Char_Index( face, prev_glyph->charcode );
            FT_Get_Kerning( face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning );
            // printf("%c(%d)-%c(%d): %ld\n",
            //       prev_glyph->charcode, prev_glyph->charcode,
            //       glyph_index, glyph_index, kerning.x);
            if( kerning.x )
            {
                kerning_t k = {prev_glyph->charcode, kerning.x / (float)(HRESf*HRESf)};
                vector_push_back( glyph->kerning, &k );
            }
        }
    }

    FT_Done_Face( face );
    FT_Done_FreeType( library );
}

// ------------------------------------------------------ texture_font_init ---
static int
texture_font_init(texture_font_t *self)
{
    FT_Library library;
    FT_Face face;
    FT_Size_Metrics metrics;

    assert(self->atlas);
    assert(self->size > 0);
    assert((self->location == TEXTURE_FONT_FILE && self->filename)
        || (self->location == TEXTURE_FONT_MEMORY
            && self->memory.base && self->memory.size));

    self->glyphs = vector_new(sizeof(texture_glyph_t *));
    self->height = 0;
    self->ascender = 0;
    self->descender = 0;
    self->outline_type = 0;
    self->outline_thickness = 0.0;
    self->hinting = 1;
    self->kerning = 1;
    self->filtering = 1;

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    self->lcd_weights[0] = 0x10;
    self->lcd_weights[1] = 0x40;
    self->lcd_weights[2] = 0x70;
    self->lcd_weights[3] = 0x40;
    self->lcd_weights[4] = 0x10;

    /* Get font metrics at high resolution */
    if (!texture_font_get_hires_face(self, &library, &face))
        return -1;

    self->underline_position = face->underline_position / (float)(HRESf*HRESf) * self->size;
    self->underline_position = round( self->underline_position );
    if( self->underline_position > -2 )
    {
        self->underline_position = -2.0;
    }

    self->underline_thickness = face->underline_thickness / (float)(HRESf*HRESf) * self->size;
    self->underline_thickness = round( self->underline_thickness );
    if( self->underline_thickness < 1 )
    {
        self->underline_thickness = 1.0;
    }

    metrics = face->size->metrics;
#if 0
    self->ascender = (metrics.ascender >> 6) / 100.0;
    self->descender = (metrics.descender >> 6) / 100.0;
    self->height = (metrics.height >> 6) / 100.0;
#endif
    self->ascender = metrics.ascender / 64.0f;
    self->descender = metrics.descender / 64.0f;
    self->height = metrics.height / 64.0f;
    self->linegap = self->height - self->ascender + self->descender;
    FT_Done_Face( face );
    FT_Done_FreeType( library );

    /* NULL is a special glyph */
    texture_font_get_glyph( self, NULL );

    return 0;
}

// --------------------------------------------- texture_font_new_from_file ---
texture_font_t *
texture_font_new_from_file(texture_atlas_t *atlas, const float pt_size,
        const char *filename)
{
    texture_font_t *self;

    assert(filename);

    self = calloc(1, sizeof(*self));
    if (!self) {
        fprintf(stderr,
                "line %d: No more memory for allocating data\n", __LINE__);
        return NULL;
    }

    self->atlas = atlas;
    self->size  = pt_size;

    self->location = TEXTURE_FONT_FILE;
    self->filename = strdup(filename);

    if (texture_font_init(self)) {
        texture_font_delete(self);
        return NULL;
    }

    return self;
}

// ------------------------------------------- texture_font_new_from_memory ---
texture_font_t *
texture_font_new_from_memory(texture_atlas_t *atlas, float pt_size,
        const void *memory_base, size_t memory_size)
{
    texture_font_t *self;

    assert(memory_base);
    assert(memory_size);

    self = calloc(1, sizeof(*self));
    if (!self) {
        fprintf(stderr,
                "line %d: No more memory for allocating data\n", __LINE__);
        return NULL;
    }

    self->atlas = atlas;
    self->size  = pt_size;

    self->location = TEXTURE_FONT_MEMORY;
    self->memory.base = memory_base;
    self->memory.size = memory_size;

    if (texture_font_init(self)) {
        texture_font_delete(self);
        return NULL;
    }

    return self;
}

// ---------------------------------------------------- texture_font_delete ---
void
texture_font_delete( texture_font_t *self )
{
    size_t i;
    texture_glyph_t *glyph;

    assert( self );

    if(self->location == TEXTURE_FONT_FILE && self->filename)
        free( self->filename );

    for( i=0; i<vector_size( self->glyphs ); ++i)
    {
        glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
        texture_glyph_delete( glyph);
    }

    vector_delete( self->glyphs );
    free( self );
}

texture_glyph_t *
texture_font_find_glyph( texture_font_t * self,
                         const char * charcode )
{
    size_t i;
    texture_glyph_t *glyph;
    uint32_t ucharcode = utf8_to_utf32( charcode );

    for( i = 0; i < self->glyphs->size; ++i )
    {
        glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
        // If charcode is -1, we don't care about outline type or thickness
        if( (glyph->charcode == ucharcode) &&
            ((ucharcode == -1) ||
             ((glyph->outline_type == self->outline_type) &&
              (glyph->outline_thickness == self->outline_thickness)) ))
        {
            return glyph;
        }
    }

    return NULL;
}

// ----------------------------------------------- texture_font_load_glyphs ---
size_t
texture_font_load_glyphs( texture_font_t * self,
                          const char * charcodes )
{
  return texture_font_load_glyphs_ex(self, charcodes, NULL, NULL);
}

size_t
texture_font_load_glyphs_ex( texture_font_t * self,
                             const char * charcodes,
                             texture_font_load_glyphs_callback callback,
                             void* callbackData )
{
    size_t i, x, y, width, height, depth, w, h;

    FT_Library library;
    FT_Error error;
    FT_Face face;
    FT_Glyph ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap ft_bitmap;
    unsigned char* ft_bitmap_buffer = NULL;
    unsigned char* ft_bitmap_buffer_outline = NULL;
    unsigned int ft_bitmap_rows = 0;
    unsigned int ft_bitmap_width = 0;
    unsigned int ft_bitmap_rows_outline = 0;
    unsigned int ft_bitmap_width_outline = 0;
    int ft_bitmap_pitch = 0;
    int ft_bitmap_pitch_outline = 0;

    FT_UInt glyph_index;
    texture_glyph_t *glyph;
    FT_Int32 flags = 0;
    int ft_glyph_top = 0;
    int ft_glyph_left = 0;
    int ft_glyph_top_outline = 0;
    int ft_glyph_left_outline = 0;
    int ft_bitmap_buffer_x = 0;
    int ft_bitmap_buffer_y = 0;
    int ft_bitmap_buffer_x_outline = 0;
    int ft_bitmap_buffer_y_outline = 0;
    FT_Pos slot_advance_x = 0;
    FT_Pos slot_advance_y = 0;
    texture_font_load_glyphs_callback_result callbackResult;

    ivec4 region;
    size_t missed = 0;

    assert( self );
    assert( charcodes );


    width  = self->atlas->width;
    height = self->atlas->height;
    depth  = self->atlas->depth;

    if (!texture_font_get_face(self, &library, &face))
        return utf8_strlen(charcodes);

    /* Load each glyph */
    for( i = 0; i < utf8_strlen(charcodes); i += utf8_surrogate_len(charcodes + i) ) {
        /* Check if charcode has been already loaded */
        if( texture_font_find_glyph( self, charcodes + i ) )
            continue;

        flags = 0;
        ft_glyph_top = 0;
        ft_glyph_left = 0;
        glyph_index = FT_Get_Char_Index( face, (FT_ULong)utf8_to_utf32( charcodes + i ) );
        // WARNING: We use texture-atlas depth to guess if user wants
        //          LCD subpixel rendering

        if(callback) {
          memset(&callbackResult, 0, sizeof(callbackResult));
        }

        if(callback && callback(callbackData, charcodes + i, &callbackResult) != 0) {
          w = callbackResult.rectW;
          h = callbackResult.rectH;
          ft_glyph_left = callbackResult.glyphX;
          ft_glyph_top = callbackResult.glyphY;
          slot_advance_x = callbackResult.advanceX;
          slot_advance_y = callbackResult.advanceY;

          region = texture_atlas_get_region( self->atlas, w + 1, h + 1 );
          if ( region.x < 0 )
          {
              missed++;
              fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
              continue;
          }
          x = region.x;
          y = region.y;

          callbackResult.rectX = region.x;
          callbackResult.rectY = region.y;

          callbackResult.mode = 1;
          callback(callbackData, charcodes + i, &callbackResult);
        }
        else {
          if( self->outline_type > 0 )
          {
              flags |= FT_LOAD_NO_BITMAP;
          }
          else
          {
              flags |= FT_LOAD_RENDER;
          }

          if( !self->hinting )
          {
              flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
          }
          else
          {
              flags |= FT_LOAD_FORCE_AUTOHINT;
          }


          if( depth == 3 )
          {
              FT_Library_SetLcdFilter( library, FT_LCD_FILTER_LIGHT );
              flags |= FT_LOAD_TARGET_LCD;
              if( self->filtering )
              {
                  FT_Library_SetLcdFilterWeights( library, self->lcd_weights );
              }
          }

          if(self->outlineMode != 0)
          {
              error = FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER );
              if( error )
              {
                  fprintf( stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                           __LINE__, FT_Errors[error].code, FT_Errors[error].message );
                  FT_Done_Face( face );
                  FT_Done_FreeType( library );
                  return utf8_strlen(charcodes) - utf8_strlen(charcodes + i);
              }

              slot            = face->glyph;
              ft_bitmap       = slot->bitmap;
              ft_glyph_top    = slot->bitmap_top;
              ft_glyph_left   = slot->bitmap_left;
              slot_advance_x  = slot->advance.x;
              slot_advance_y  = slot->advance.y;

              ft_bitmap_pitch = ft_bitmap.pitch;
              ft_bitmap_rows = ft_bitmap.rows;
              ft_bitmap_width = ft_bitmap.width;
              if(ft_bitmap.buffer) {
                ft_bitmap_buffer = malloc(ft_bitmap_pitch * ft_bitmap_rows);
                memcpy(ft_bitmap_buffer, ft_bitmap.buffer, ft_bitmap_pitch * ft_bitmap_rows);
              }

              error = FT_Load_Glyph( face, glyph_index, flags );
              if( error )
              {
                  fprintf( stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                           __LINE__, FT_Errors[error].code, FT_Errors[error].message );
                  FT_Done_Face( face );
                  FT_Done_FreeType( library );
                  return utf8_strlen(charcodes) - utf8_strlen(charcodes + i);
              }

              if(self->outlineMode == 1)
              {
                  FT_Stroker stroker;
                  FT_BitmapGlyph ft_bitmap_glyph;
                  error = FT_Stroker_New( library, &stroker );
                  if( error )
                  {
                      fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                              FT_Errors[error].code, FT_Errors[error].message);
                      FT_Done_Face( face );
                      FT_Stroker_Done( stroker );
                      FT_Done_FreeType( library );
                      return 0;
                  }
                  FT_Stroker_Set(stroker,
                                  (int)(self->outline_thickness * HRES),
                                  FT_STROKER_LINECAP_ROUND,
                                  FT_STROKER_LINEJOIN_ROUND,
                                  0);
                  error = FT_Get_Glyph( face->glyph, &ft_glyph);
                  if( error )
                  {
                      fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                              FT_Errors[error].code, FT_Errors[error].message);
                      FT_Done_Face( face );
                      FT_Stroker_Done( stroker );
                      FT_Done_FreeType( library );
                      return 0;
                  }

                  if( self->outline_type == 1 || self->outline_type == 0 )
                  {
                      error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
                  }
                  else if ( self->outline_type == 2 )
                  {
                      error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
                  }
                  else if ( self->outline_type == 3 )
                  {
                      error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );
                  }
                  if( error )
                  {
                      fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                              FT_Errors[error].code, FT_Errors[error].message);
                      FT_Done_Face( face );
                      FT_Stroker_Done( stroker );
                      FT_Done_FreeType( library );
                      return 0;
                  }

                  FT_Glyph ft_glyph2;
                  error = FT_Glyph_Copy( ft_glyph, &ft_glyph2 );
                  if( error )
                  {
                      fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                              FT_Errors[error].code, FT_Errors[error].message);
                  }

                  error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                  if( error )
                  {
                      fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                              FT_Errors[error].code, FT_Errors[error].message);
                      FT_Done_Face( face );
                      FT_Stroker_Done( stroker );
                      FT_Done_FreeType( library );
                      return 0;
                  }

                  FT_Vector delta;
                  delta.x = 0;
                  delta.y = 0;

                  if(ft_bitmap_width_outline > ft_bitmap_width) {
                    int odd = (ft_bitmap_width_outline - ft_bitmap_width) & 1;
                    if(odd) {
                      delta.x = -32;
                    }
                  }

                  if(ft_bitmap_rows_outline > ft_bitmap_rows) {
                    int odd = (ft_bitmap_rows_outline - ft_bitmap_rows) & 1;
                    if(odd) {
                      delta.y = -32;
                    }
                  }

                  if(delta.x != 0 || delta.y != 0) {
                    error = FT_Glyph_Transform(ft_glyph2, NULL, &delta);
                    if( error )
                    {
                        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                                FT_Errors[error].code, FT_Errors[error].message);
                    }

                    error = FT_Glyph_To_Bitmap( &ft_glyph2, FT_RENDER_MODE_NORMAL, 0, 1);
                    if( error )
                    {
                        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                                FT_Errors[error].code, FT_Errors[error].message);
                        FT_Done_Face( face );
                        FT_Stroker_Done( stroker );
                        FT_Done_FreeType( library );
                        return 0;
                    }

                    ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph2;
                  }
                  else {
                    ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
                  }
                  ft_bitmap_pitch_outline = ft_bitmap_glyph->bitmap.pitch;
                  ft_bitmap_rows_outline = ft_bitmap_glyph->bitmap.rows;
                  ft_bitmap_width_outline = ft_bitmap_glyph->bitmap.width;

                  ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;

                  ft_bitmap_pitch_outline = ft_bitmap_glyph->bitmap.pitch;
                  ft_bitmap_rows_outline = ft_bitmap_glyph->bitmap.rows;
                  ft_bitmap_width_outline = ft_bitmap_glyph->bitmap.width;
                  if(ft_bitmap_glyph->bitmap.buffer) {
                    ft_bitmap_buffer_outline = malloc(ft_bitmap_pitch_outline * ft_bitmap_rows_outline);
                    memcpy(ft_bitmap_buffer_outline, ft_bitmap_glyph->bitmap.buffer, ft_bitmap_pitch_outline * ft_bitmap_rows_outline);
                  }
                  ft_glyph_top_outline      = ft_bitmap_glyph->top;
                  ft_glyph_left_outline     = ft_bitmap_glyph->left;
                  ft_bitmap_buffer_x        = (ft_glyph_left > ft_glyph_left_outline) ? (ft_glyph_left - ft_glyph_left_outline) : 0;
                  ft_bitmap_buffer_y        = (ft_glyph_top_outline > ft_glyph_top) ? (ft_glyph_top_outline - ft_glyph_top) : 0;
                  ft_glyph_top              = ft_glyph_top_outline + self->outline_thickness;
                  ft_glyph_left             = ft_glyph_left_outline + self->outline_thickness;

                  slot_advance_x += (FT_Pos) (self->outline_thickness * HRESf);

                  FT_Stroker_Done(stroker);
                  FT_Done_Glyph( ft_glyph2 );
                  FT_Done_Glyph( ft_glyph );
              }
          }
          else {
            error = FT_Load_Glyph( face, glyph_index, flags );
            if( error )
            {
                fprintf( stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                         __LINE__, FT_Errors[error].code, FT_Errors[error].message );
                FT_Done_Face( face );
                FT_Done_FreeType( library );
                return utf8_strlen(charcodes) - utf8_strlen(charcodes + i);
            }

            if( self->outline_type == 0 )
            {
                slot            = face->glyph;
                ft_bitmap       = slot->bitmap;
                ft_glyph_top    = slot->bitmap_top;
                ft_glyph_left   = slot->bitmap_left;

                ft_bitmap_buffer = ft_bitmap.buffer;
                ft_bitmap_pitch = ft_bitmap.pitch;
                ft_bitmap_rows = ft_bitmap.rows;
                ft_bitmap_width = ft_bitmap.width;

                slot_advance_x  = slot->advance.x;
                slot_advance_y  = slot->advance.y;
            }
            else
            {
                FT_Stroker stroker;
                FT_BitmapGlyph ft_bitmap_glyph;
                error = FT_Stroker_New( library, &stroker );
                if( error )
                {
                    fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                            FT_Errors[error].code, FT_Errors[error].message);
                    FT_Done_Face( face );
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
                    return 0;
                }
                FT_Stroker_Set(stroker,
                                (int)(self->outline_thickness * HRES),
                                FT_STROKER_LINECAP_ROUND,
                                FT_STROKER_LINEJOIN_ROUND,
                                0);
                error = FT_Get_Glyph( face->glyph, &ft_glyph);
                if( error )
                {
                    fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                            FT_Errors[error].code, FT_Errors[error].message);
                    FT_Done_Face( face );
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
                    return 0;
                }

                if( self->outline_type == 1 )
                {
                    error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
                }
                else if ( self->outline_type == 2 )
                {
                    error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
                }
                else if ( self->outline_type == 3 )
                {
                    error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );
                }
                if( error )
                {
                    fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                            FT_Errors[error].code, FT_Errors[error].message);
                    FT_Done_Face( face );
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
                    return 0;
                }

                if( depth == 1)
                {
                    error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                    if( error )
                    {
                        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                                FT_Errors[error].code, FT_Errors[error].message);
                        FT_Done_Face( face );
                        FT_Stroker_Done( stroker );
                        FT_Done_FreeType( library );
                        return 0;
                    }
                }
                else
                {
                    error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
                    if( error )
                    {
                        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                                FT_Errors[error].code, FT_Errors[error].message);
                        FT_Done_Face( face );
                        FT_Stroker_Done( stroker );
                        FT_Done_FreeType( library );
                        return 0;
                    }
                }
                ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
                ft_bitmap       = ft_bitmap_glyph->bitmap;
                ft_glyph_top    = ft_bitmap_glyph->top;
                ft_glyph_left   = ft_bitmap_glyph->left;

                slot_advance_x  = ft_glyph->advance.x;
                slot_advance_y  = ft_glyph->advance.y;

                FT_Stroker_Done(stroker);
                FT_Done_Glyph( ft_glyph );

                ft_bitmap_buffer = ft_bitmap.buffer;
                ft_bitmap_pitch = ft_bitmap.pitch;
                ft_bitmap_rows = ft_bitmap.rows;
                ft_bitmap_width = ft_bitmap.width;
            }
          }

          if(self->outlineMode != 0) {
            // We want each glyph to be separated by at least one black pixel
            // (for example for shader used in demo-subpixel.c)
            w = ft_bitmap_width;
            h = ft_bitmap_rows;

            if(self->outlineMode != 0) {
              if(w < ft_bitmap_width_outline) {
                w = ft_bitmap_width_outline;
              }
              if(h < ft_bitmap_rows_outline) {
                h = ft_bitmap_rows_outline;
              }
            }

            region = texture_atlas_get_region( self->atlas, w + 1, h + 1);
            if ( region.x < 0 )
            {
                missed++;
                fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
                continue;
            }
            x = region.x;
            y = region.y;

            if(ft_bitmap_buffer) {
              texture_atlas_set_region( self->atlas, x + ft_bitmap_buffer_x, y + ft_bitmap_buffer_y, ft_bitmap_width, ft_bitmap_rows,
                                        ft_bitmap_buffer, ft_bitmap_pitch );

              free(ft_bitmap_buffer);
              ft_bitmap_buffer = NULL;
            }

            if(self->outlineMode != 0) {
              if(ft_bitmap_buffer_outline) {
                texture_atlas_set_region_outline( self->atlas, x + ft_bitmap_buffer_x_outline, y + ft_bitmap_buffer_y_outline, ft_bitmap_width_outline, ft_bitmap_rows_outline,
                                          ft_bitmap_buffer_outline, ft_bitmap_pitch_outline );

                free(ft_bitmap_buffer_outline);
                ft_bitmap_buffer_outline = NULL;
              }
            }
          }
          else {
            // We want each glyph to be separated by at least one black pixel
            // (for example for shader used in demo-subpixel.c)
            w = ft_bitmap_width/depth + 1;
            h = ft_bitmap_rows + 1;
            region = texture_atlas_get_region( self->atlas, w, h );
            if ( region.x < 0 )
            {
                missed++;
                fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
                continue;
            }
            w = w - 1;
            h = h - 1;
            x = region.x;
            y = region.y;
            texture_atlas_set_region( self->atlas, x, y, w, h,
                                      ft_bitmap_buffer, ft_bitmap_pitch );
          }
        }

        glyph = texture_glyph_new( );
        glyph->charcode = utf8_to_utf32( charcodes + i );
        glyph->width    = w;
        glyph->height   = h;
        glyph->outline_type = self->outline_type;
        glyph->outline_thickness = self->outline_thickness;
        glyph->offset_x = ft_glyph_left;
        glyph->offset_y = ft_glyph_top;
        glyph->s0       = x/(float)width;
        glyph->t0       = y/(float)height;
        glyph->s1       = (x + glyph->width)/(float)width;
        glyph->t1       = (y + glyph->height)/(float)height;
        glyph->advance_x = slot_advance_x / HRESf;
        glyph->advance_y = slot_advance_y / HRESf;

        vector_push_back( self->glyphs, &glyph );
    }

    FT_Done_Face( face );
    FT_Done_FreeType( library );
#if 0
    texture_atlas_upload( self->atlas );
#endif
    if(self->kerning)
      texture_font_generate_kerning( self );
    return missed;
}


// ------------------------------------------------- texture_font_get_glyph ---
texture_glyph_t *
texture_font_get_glyph( texture_font_t * self,
                        const char * charcode )
{
    texture_glyph_t *glyph;

    assert( self );
    assert( self->filename );
    assert( self->atlas );

    /* Check if charcode has been already loaded */
    if( (glyph = texture_font_find_glyph( self, charcode )) )
        return glyph;

    /* charcode NULL is special : it is used for line drawing (overline,
     * underline, strikethrough) and background.
     */
    if( !charcode )
    {
        size_t width  = self->atlas->width;
        size_t height = self->atlas->height;
        ivec4 region = texture_atlas_get_region( self->atlas, 5, 5 );
        texture_glyph_t * glyph = texture_glyph_new( );
        static unsigned char data[4*4*3] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
        if ( region.x < 0 )
        {
            fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
            return NULL;
        }
        texture_atlas_set_region( self->atlas, region.x, region.y, 4, 4, data, 0 );
        glyph->charcode = -1;
        glyph->s0 = (region.x+2)/(float)width;
        glyph->t0 = (region.y+2)/(float)height;
        glyph->s1 = (region.x+3)/(float)width;
        glyph->t1 = (region.y+3)/(float)height;
        vector_push_back( self->glyphs, &glyph );
        return glyph; //*(texture_glyph_t **) vector_back( self->glyphs );
    }

    /* Glyph has not been already loaded */
    if( texture_font_load_glyphs( self, charcode ) == 0 )
    {
        return texture_font_find_glyph( self, charcode );
    }
    return NULL;
}
