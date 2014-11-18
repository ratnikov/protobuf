#!/usr/bin/ruby

require 'mkmf'

$CFLAGS += " -O3 -std=c99 -Wno-unused-function -DNDEBUG"

upb_base = "../../../../../third_party/upb"

find_header("upb/upb.h", upb_base) or
  raise "Can't find upb headers"
find_library("upb_pic", "upb_msgdef_new", upb_base + "/lib") or
  raise "Can't find upb lib"
find_library("upb.descriptor_pic", "upb_descreader_init", upb_base + "/lib") or
  raise "Can't find upb.descriptor lib"
find_library("upb.pb_pic", "upb_pbdecoder_init", upb_base + "/lib") or
  raise "Can't find upb.pb lib"

$objs = ["protobuf.o", "defs.o", "storage.o", "message.o",
         "repeated_field.o", "encode_decode.o"]

create_makefile("protobuf")
