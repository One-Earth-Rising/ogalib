////////////////////////////////////////////////////////////////////////////////
//
// Prime Engine
// Copyright 2011-2024 Sean Reid
//
// This file and/or data is a part of Prime Engine.  Prime Engine is owned by
// Sean Reid.  A valid license is required to use Prime Engine in any form
// including, but not limited to, commercial and/or educational use.
// For more information about Prime Engine, visit:
//
//   http://seanreid.ca
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  //! Use DXT1 compression.
  squishc_flag_kDxt1 = ( 1 << 0 ),

  //! Use DXT3 compression.
  squishc_flag_kDxt3 = ( 1 << 1 ),

  //! Use DXT5 compression.
  squishc_flag_kDxt5 = ( 1 << 2 ),

  //! Use a very slow but very high quality colour compressor.
  squishc_flag_kColourIterativeClusterFit = ( 1 << 8 ),

  //! Use a slow but high quality colour compressor (the default).
  squishc_flag_kColourClusterFit = ( 1 << 3 ),

  //! Use a fast but low quality colour compressor.
  squishc_flag_kColourRangeFit = ( 1 << 4 ),

  //! Use a perceptual metric for colour error (the default).
  squishc_flag_kColourMetricPerceptual = ( 1 << 5 ),

  //! Use a uniform metric for colour error.
  squishc_flag_kColourMetricUniform = ( 1 << 6 ),

  //! Weight the colour by alpha during cluster fit (disabled by default).
  squishc_flag_kWeightColourByAlpha = ( 1 << 7 )
} squishc_flag;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

void squishc_compress(const unsigned char* pixels, void* block, int flags);
void squishc_decompress(unsigned char* pixels, const void* block, int flags);

#ifdef __cplusplus
};
#endif
