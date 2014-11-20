// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__
#define __GOOGLE_PROTOBUF_RUBY_PROTOBUF_H__

#include <ruby/ruby.h>
#include <ruby/vm.h>

#include "upb/def.h"
#include "upb/handlers.h"
#include "upb/pb/decoder.h"
#include "upb/pb/encoder.h"
#include "upb/pb/glue.h"
#include "upb/shim/shim.h"
#include "upb/symtab.h"

// Forward decls.
struct SymbolTable;
struct MessageDef;
struct FieldDef;
struct EnumDef;
struct MessageLayout;
struct MessageHeader;
struct MessageBuilderContext;
struct EnumBuilderContext;
struct Builder;

typedef struct SymbolTable SymbolTable;
typedef struct MessageDef MessageDef;
typedef struct FieldDef FieldDef;
typedef struct EnumDef EnumDef;
typedef struct MessageLayout MessageLayout;
typedef struct MessageHeader MessageHeader;
typedef struct MessageBuilderContext MessageBuilderContext;
typedef struct EnumBuilderContext EnumBuilderContext;
typedef struct Builder Builder;

// -----------------------------------------------------------------------------
// Ruby class structure definitions.
// -----------------------------------------------------------------------------

struct SymbolTable {
  VALUE _value;
  upb_symtab* symtab;
};

struct MessageDef {
  VALUE _value;
  upb_msgdef* msgdef;
  MessageLayout* layout;
  VALUE klass;  // begins as nil
  VALUE fields;  // Ruby array of FieldDef Ruby objects
  VALUE field_map;  // Ruby hashmap from field name to FieldDef Ruby object
  const upb_pbdecodermethod* fill_method;
  const upb_handlers* serialize_handlers;
};

struct FieldDef {
  VALUE _value;
  upb_fielddef* fielddef;
};

struct EnumDef {
  VALUE _value;
  upb_enumdef* enumdef;
  VALUE module;  // begins as nil
};

struct MessageBuilderContext {
  VALUE _value;
  VALUE msgdef;
};

struct EnumBuilderContext {
  VALUE _value;
  VALUE enumdef;
};

struct Builder {
  VALUE _value;
  VALUE pending_list;
};

extern VALUE cSymbolTable;
extern VALUE cMessageDef;
extern VALUE cFieldDef;
extern VALUE cEnumDef;
extern VALUE cMessageBuilderContext;
extern VALUE cEnumBuilderContext;
extern VALUE cBuilder;

void SymbolTable_mark(void* _self);
void SymbolTable_free(void* _self);
VALUE SymbolTable_alloc(VALUE klass);
void SymbolTable_register(VALUE module);
SymbolTable* ruby_to_SymbolTable(VALUE value);
VALUE SymbolTable_add(VALUE _self, VALUE def);
VALUE SymbolTable_build(VALUE _self);
VALUE SymbolTable_lookup(VALUE _self, VALUE name);
VALUE SymbolTable_get_class(VALUE _self, VALUE name);
VALUE SymbolTable_get_enum(VALUE _self, VALUE name);
VALUE SymbolTable_global_symtab(VALUE _self);

void MessageDef_mark(void* _self);
void MessageDef_free(void* _self);
VALUE MessageDef_alloc(VALUE klass);
void MessageDef_register(VALUE module);
MessageDef* ruby_to_MessageDef(VALUE value);
VALUE MessageDef_name(VALUE _self);
VALUE MessageDef_name_set(VALUE _self, VALUE str);
VALUE MessageDef_fields(VALUE _self);
VALUE MessageDef_lookup(VALUE _self, VALUE name);
VALUE MessageDef_add_field(VALUE _self, VALUE obj);
VALUE MessageDef_msgclass(VALUE _self);
extern const rb_data_type_t _MessageDef_type;

void FieldDef_mark(void* _self);
void FieldDef_free(void* _self);
VALUE FieldDef_alloc(VALUE klass);
void FieldDef_register(VALUE module);
FieldDef* ruby_to_FieldDef(VALUE value);
VALUE FieldDef_name(VALUE _self);
VALUE FieldDef_name_set(VALUE _self, VALUE str);
VALUE FieldDef_type(VALUE _self);
VALUE FieldDef_type_set(VALUE _self, VALUE type);
VALUE FieldDef_label(VALUE _self);
VALUE FieldDef_label_set(VALUE _self, VALUE label);
VALUE FieldDef_number(VALUE _self);
VALUE FieldDef_number_set(VALUE _self, VALUE number);
VALUE FieldDef_submsg_name(VALUE _self);
VALUE FieldDef_submsg_name_set(VALUE _self, VALUE value);
VALUE FieldDef_subtype(VALUE _self);
VALUE FieldDef_get(VALUE _self, VALUE msg_rb);
VALUE FieldDef_set(VALUE _self, VALUE msg_rb, VALUE value);
upb_fieldtype_t ruby_to_fieldtype(VALUE type);
VALUE fieldtype_to_ruby(upb_fieldtype_t type);

void EnumDef_mark(void* _self);
void EnumDef_free(void* _self);
VALUE EnumDef_alloc(VALUE klass);
void EnumDef_register(VALUE module);
EnumDef* ruby_to_EnumDef(VALUE value);
VALUE EnumDef_name(VALUE _self);
VALUE EnumDef_name_set(VALUE _self, VALUE str);
VALUE EnumDef_add_value(VALUE _self, VALUE name, VALUE number);
VALUE EnumDef_lookup_name(VALUE _self, VALUE name);
VALUE EnumDef_lookup_value(VALUE _self, VALUE number);
VALUE EnumDef_values(VALUE _self);
VALUE EnumDef_enummodule(VALUE _self);
extern const rb_data_type_t _EnumDef_type;

void MessageBuilderContext_mark(void* _self);
void MessageBuilderContext_free(void* _self);
VALUE MessageBuilderContext_alloc(VALUE klass);
void MessageBuilderContext_register(VALUE module);
MessageBuilderContext* ruby_to_MessageBuilderContext(VALUE value);
VALUE MessageBuilderContext_initialize(VALUE _self, VALUE msgdef);
VALUE MessageBuilderContext_optional(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_required(int argc, VALUE* argv, VALUE _self);
VALUE MessageBuilderContext_repeated(int argc, VALUE* argv, VALUE _self);

void EnumBuilderContext_mark(void* _self);
void EnumBuilderContext_free(void* _self);
VALUE EnumBuilderContext_alloc(VALUE klass);
void EnumBuilderContext_register(VALUE module);
EnumBuilderContext* ruby_to_EnumBuilderContext(VALUE value);
VALUE EnumBuilderContext_initialize(VALUE _self, VALUE enumdef);
VALUE EnumBuilderContext_value(VALUE _self, VALUE name, VALUE number);

void Builder_mark(void* _self);
void Builder_free(void* _self);
VALUE Builder_alloc(VALUE klass);
void Builder_register(VALUE module);
Builder* ruby_to_Builder(VALUE value);
VALUE Builder_add_message(VALUE _self, VALUE name);
VALUE Builder_add_enum(VALUE _self, VALUE name);
VALUE Builder_finalize_to_symtab(VALUE _self, VALUE symtab_rb);

// -----------------------------------------------------------------------------
// Native slot storage abstraction.
// -----------------------------------------------------------------------------

size_t native_slot_size(upb_fieldtype_t type);
void native_slot_set(upb_fieldtype_t type,
                     VALUE type_class,
                     void* memory,
                     VALUE value);
VALUE native_slot_get(upb_fieldtype_t type,
                      VALUE type_class,
                      void* memory);
void native_slot_init(upb_fieldtype_t type, void* memory);
void native_slot_mark(upb_fieldtype_t type, void* memory);
void native_slot_dup(upb_fieldtype_t type, void* to, void* from);
bool native_slot_eq(upb_fieldtype_t type, void* mem1, void* mem2);

// -----------------------------------------------------------------------------
// Repeated field container type.
// -----------------------------------------------------------------------------

typedef struct {
  upb_fieldtype_t field_type;
  VALUE field_type_class;
  void* elements;
  int size;
  int capacity;
} RepeatedField;

void RepeatedField_mark(void* self);
void RepeatedField_free(void* self);
VALUE RepeatedField_alloc(VALUE klass);
VALUE RepeatedField_init(int argc, VALUE* argv, VALUE self);
void RepeatedField_register(VALUE module);

extern const rb_data_type_t RepeatedField_type;
extern VALUE cRepeatedField;

RepeatedField* ruby_to_RepeatedField(VALUE value);

void RepeatedField_register(VALUE module);
VALUE RepeatedField_each(VALUE _self);
VALUE RepeatedField_index(VALUE _self, VALUE _index);
void* RepeatedField_index_native(VALUE _self, int index);
VALUE RepeatedField_index_set(VALUE _self, VALUE _index, VALUE val);
void RepeatedField_reserve(RepeatedField* self, int new_size);
VALUE RepeatedField_push(VALUE _self, VALUE val);
void RepeatedField_push_native(VALUE _self, void* data);
VALUE RepeatedField_pop(VALUE _self);
VALUE RepeatedField_insert(int argc, VALUE* argv, VALUE _self);
VALUE RepeatedField_length(VALUE _self);
VALUE RepeatedField_dup(VALUE _self);
VALUE RepeatedField_eq(VALUE _self, VALUE _other);
VALUE RepeatedField_hash(VALUE _self);
VALUE RepeatedField_inspect(VALUE _self);
VALUE RepeatedField_plus(VALUE _self, VALUE list);

// -----------------------------------------------------------------------------
// Message layout / storage.
// -----------------------------------------------------------------------------

struct MessageLayout {
  const upb_msgdef* msgdef;
  size_t* offsets;
  size_t size;
};

MessageLayout* create_layout(const upb_msgdef* msgdef);
void free_layout(MessageLayout* layout);
VALUE layout_get(MessageLayout* layout,
                 void* storage,
                 const upb_fielddef* field);
void layout_set(MessageLayout* layout,
                void* storage,
                const upb_fielddef* field,
                VALUE val);
void layout_init(MessageLayout* layout, void* storage);
void layout_mark(MessageLayout* layout, void* storage);
void layout_dup(MessageLayout* layout, void* to, void* from);
VALUE layout_eq(MessageLayout* layout, void* msg1, void* msg2);
VALUE layout_hash(MessageLayout* layout, void* storage);
VALUE layout_inspect(MessageLayout* layout, void* storage);

// -----------------------------------------------------------------------------
// Message class creation.
// -----------------------------------------------------------------------------

struct MessageHeader {
  VALUE msgdef_rb;
  MessageDef* msgdef;  // kept alive by msgdef_rb reference.
  // Data comes after this.
};

extern rb_data_type_t Message_type;

VALUE build_class_from_msgdef(MessageDef* msgdef);
void* Message_data(void* msg);
void Message_mark(void* self);
void Message_free(void* self);
VALUE Message_alloc(VALUE klass);
VALUE Message_method_missing(int argc, VALUE* argv, VALUE _self);
VALUE Message_initialize(int argc, VALUE* argv, VALUE _self);
VALUE Message_dup(VALUE _self);
VALUE Message_eq(VALUE _self, VALUE _other);
VALUE Message_hash(VALUE _self);
VALUE Message_inspect(VALUE _self);
VALUE Message_descriptor(VALUE klass);
VALUE Message_decode(VALUE klass, VALUE data);
VALUE Message_encode(VALUE klass, VALUE msg_rb);

VALUE build_module_from_enumdef(EnumDef* enumdef);
VALUE enum_lookup(VALUE self, VALUE number);
VALUE enum_resolve(VALUE self, VALUE sym);

const upb_pbdecodermethod *new_fillmsg_decodermethod(
    MessageDef* msgdef, const void *owner);

// -----------------------------------------------------------------------------
// Global map from upb {msg,enum}defs to wrapper MessageDef/EnumDef instances.
// -----------------------------------------------------------------------------
void add_def_obj(void* def, VALUE value);
VALUE get_def_obj(void* def);

#endif  // EXPERIMENTAL_USERS_CFALLIN_PROTO3_RUBY_C_EXT_UPB_H_
