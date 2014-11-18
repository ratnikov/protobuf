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

#include "protobuf.h"

// -----------------------------------------------------------------------------
// SymbolTable.
// -----------------------------------------------------------------------------

#define DEFINE_CLASS(name, string_name)                             \
    VALUE c ## name;                                                \
    const rb_data_type_t _ ## name ## _type = {                     \
      string_name,                                                  \
      { name ## _mark, name ## _free, NULL },                       \
    };                                                              \
    name* ruby_to_ ## name(VALUE val) {                             \
      name* ret;                                                    \
      TypedData_Get_Struct(val, name, &_ ## name ## _type, ret);    \
      return ret;                                                   \
    }                                                               \

#define SELF(type)                                                  \
    type* self = ruby_to_ ## type(_self);

// Global singleton SymbolTable. The user is free to create others, but this is
// used by generated code.
VALUE global_symtab;

DEFINE_CLASS(SymbolTable, "Google::Protobuf::SymbolTable");

void SymbolTable_mark(void* _self) {
}

void SymbolTable_free(void* _self) {
  SymbolTable* self = _self;
  upb_symtab_unref(self->symtab, &self->symtab);
  xfree(self);
}

VALUE SymbolTable_alloc(VALUE klass) {
  SymbolTable* self = ALLOC(SymbolTable);
  self->symtab = upb_symtab_new(&self->symtab);
  self->_value = TypedData_Wrap_Struct(klass, &_SymbolTable_type, self);
  return self->_value;
}

void SymbolTable_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "SymbolTable", rb_cObject);
  rb_define_alloc_func(klass, SymbolTable_alloc);
  rb_define_method(klass, "add", SymbolTable_add, 2);
  rb_define_method(klass, "build", SymbolTable_build, 0);
  rb_define_method(klass, "lookup", SymbolTable_lookup, 1);
  rb_define_method(klass, "get_class", SymbolTable_get_class, 1);
  rb_define_method(klass, "get_enum", SymbolTable_get_enum, 1);
  rb_define_singleton_method(klass, "global_symtab",
                             SymbolTable_global_symtab, 0);
  cSymbolTable = klass;
  rb_gc_register_address(&cSymbolTable);

  global_symtab = rb_class_new_instance(0, NULL, klass);
  rb_gc_register_address(&global_symtab);
}

static void add_msgdef_to_symtab(SymbolTable* self, MessageDef* msgdef) {
  upb_status s = UPB_STATUS_INIT;
  upb_symtab_add(self->symtab, (upb_def**)&msgdef->msgdef, 1, NULL, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Adding MessageDef to SymbolTable failed.");
  }
  add_def_obj(msgdef->msgdef, msgdef->_value);
}

static void add_enumdef_to_symtab(SymbolTable* self, EnumDef* enumdef) {
  upb_status s = UPB_STATUS_INIT;
  upb_symtab_add(self->symtab, (upb_def**)&enumdef->enumdef, 1, NULL, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Adding EnumDef to SymbolTable failed.");
  }
  add_def_obj(enumdef->enumdef, enumdef->_value);
}

VALUE SymbolTable_add(VALUE _self, VALUE def) {
  SELF(SymbolTable);
  VALUE def_klass = rb_obj_class(def);
  if (def_klass == cMessageDef) {
    add_msgdef_to_symtab(self, ruby_to_MessageDef(def));
  } else if (def_klass == cEnumDef) {
    add_enumdef_to_symtab(self, ruby_to_EnumDef(def));
  } else {
    rb_raise(rb_eArgError,
             "Second argument must be a MessageDef or EnumDef.");
  }
  return Qnil;
}

VALUE SymbolTable_build(VALUE _self) {
  VALUE ctx = rb_class_new_instance(0, NULL, cBuilder);
  VALUE block = rb_block_proc();
  rb_funcall_with_block(ctx, rb_intern("instance_eval"), 0, NULL, block);
  rb_funcall(ctx, rb_intern("finalize_to_symtab"), 1, _self);
  return Qnil;
}

static const char* get_str(VALUE str) {
  if (TYPE(str) != T_STRING) {
    rb_raise(rb_eArgError, "String expected.");
  }
  return RSTRING_PTR(str);
}

VALUE SymbolTable_lookup(VALUE _self, VALUE name) {
  SELF(SymbolTable);
  const char* name_str = get_str(name);
  const upb_def* def = upb_symtab_lookup(self->symtab, name_str);
  if (!def) {
    return Qnil;
  }
  return get_def_obj((void*)def);
}

VALUE SymbolTable_get_class(VALUE _self, VALUE name) {
  VALUE def_rb = SymbolTable_lookup(_self, name);
  if (def_rb == Qnil) {
    return Qnil;
  }
  if (CLASS_OF(def_rb) != cMessageDef) {
    rb_raise(rb_eTypeError, "Name does not name a message type.");
  }
  MessageDef* msgdef = ruby_to_MessageDef(def_rb);
  if (msgdef->klass == Qnil) {
    msgdef->klass = build_class_from_msgdef(msgdef);
  }
  return msgdef->klass;
}

VALUE SymbolTable_get_enum(VALUE _self, VALUE name) {
  VALUE def_rb = SymbolTable_lookup(_self, name);
  if (def_rb == Qnil) {
    return Qnil;
  }
  if (CLASS_OF(def_rb) != cEnumDef) {
    rb_raise(rb_eTypeError, "Name does not name an enum type.");
  }
  EnumDef* enumdef = ruby_to_EnumDef(def_rb);
  if (enumdef->module == Qnil) {
    enumdef->module = build_module_from_enumdef(enumdef);
  }
  return enumdef->module;
}

VALUE SymbolTable_global_symtab(VALUE _self) {
  return global_symtab;
}

// -----------------------------------------------------------------------------
// MessageDef.
// -----------------------------------------------------------------------------

DEFINE_CLASS(MessageDef, "Google::Protobuf::MessageDef");

void MessageDef_mark(void* _self) {
  MessageDef* self = _self;
  rb_gc_mark(self->klass);
  rb_gc_mark(self->fields);
}

void MessageDef_free(void* _self) {
  MessageDef* self = _self;
  upb_msgdef_unref(self->msgdef, &self->msgdef);
  if (self->layout) {
    free_layout(self->layout);
  }
  if (self->fill_method) {
    upb_pbdecodermethod_unref(self->fill_method, &self->fill_method);
  }
  if (self->serialize_handlers) {
    upb_handlers_unref(self->serialize_handlers, &self->serialize_handlers);
  }
  xfree(self);
}

VALUE MessageDef_alloc(VALUE klass) {
  MessageDef* self = ALLOC(MessageDef);
  self->_value = TypedData_Wrap_Struct(klass, &_MessageDef_type, self);
  self->msgdef = upb_msgdef_new(&self->msgdef);
  self->klass = Qnil;
  self->fields = rb_ary_new();
  self->layout = NULL;
  self->fill_method = NULL;
  self->serialize_handlers = NULL;
  return self->_value;
}

void MessageDef_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "MessageDef", rb_cObject);
  rb_define_alloc_func(klass, MessageDef_alloc);
  rb_define_method(klass, "fields", MessageDef_fields, 0);
  rb_define_method(klass, "add_field", MessageDef_add_field, 1);
  rb_define_method(klass, "name", MessageDef_name, 0);
  rb_define_method(klass, "name=", MessageDef_name_set, 1);
  cMessageDef = klass;
  rb_gc_register_address(&cMessageDef);
}

VALUE MessageDef_name(VALUE _self) {
  SELF(MessageDef);
  const char* s = upb_msgdef_fullname(self->msgdef);
  if (s == NULL) {
    s = "";
  }
  return rb_str_new2(s);
}

VALUE MessageDef_name_set(VALUE _self, VALUE str) {
  SELF(MessageDef);
  const char* name = get_str(str);
  upb_status s = UPB_STATUS_INIT;
  upb_msgdef_setfullname(self->msgdef, name, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Error setting MessageDef name.");
  }
  return Qnil;
}

VALUE MessageDef_fields(VALUE _self) {
  SELF(MessageDef);
  return self->fields;
}

VALUE MessageDef_add_field(VALUE _self, VALUE obj) {
  SELF(MessageDef);
  VALUE obj_klass = rb_obj_class(obj);
  if (obj_klass != cFieldDef) {
    rb_raise(rb_eTypeError, "add_field expects a FieldDef instance.");
  }
  FieldDef* def = ruby_to_FieldDef(obj);
  upb_status s = UPB_STATUS_INIT;
  upb_msgdef_addfield(self->msgdef, def->fielddef, NULL, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Adding field to MessageDef failed.");
  }
  rb_ary_push(self->fields, obj);
  return Qnil;
}

// -----------------------------------------------------------------------------
// FieldDef.
// -----------------------------------------------------------------------------

DEFINE_CLASS(FieldDef, "Google::Protobuf::FieldDef");

void FieldDef_mark(void* _self) {
}

void FieldDef_free(void* _self) {
  FieldDef* self = _self;
  upb_fielddef_unref(self->fielddef, &self->fielddef);
  xfree(self);
}

VALUE FieldDef_alloc(VALUE klass) {
  FieldDef* self = ALLOC(FieldDef);
  self->_value = TypedData_Wrap_Struct(klass, &_FieldDef_type, self);
  self->fielddef = upb_fielddef_new(&self->fielddef);
  upb_fielddef_setpacked(self->fielddef, false);
  return self->_value;
}

void FieldDef_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "FieldDef", rb_cObject);
  rb_define_alloc_func(klass, FieldDef_alloc);
  rb_define_method(klass, "name", FieldDef_name, 0);
  rb_define_method(klass, "name=", FieldDef_name_set, 1);
  rb_define_method(klass, "type", FieldDef_type, 0);
  rb_define_method(klass, "type=", FieldDef_type_set, 1);
  rb_define_method(klass, "label", FieldDef_label, 0);
  rb_define_method(klass, "label=", FieldDef_label_set, 1);
  rb_define_method(klass, "number", FieldDef_number, 0);
  rb_define_method(klass, "number=", FieldDef_number_set, 1);
  rb_define_method(klass, "submsg_name", FieldDef_submsg_name, 0);
  rb_define_method(klass, "submsg_name=", FieldDef_submsg_name_set, 1);
  cFieldDef = klass;
  rb_gc_register_address(&cFieldDef);
}

VALUE FieldDef_name(VALUE _self) {
  SELF(FieldDef);
  const char* s = upb_fielddef_name(self->fielddef);
  if (s == NULL) {
    s = "";
  }
  return rb_str_new2(s);
}

VALUE FieldDef_name_set(VALUE _self, VALUE str) {
  SELF(FieldDef);
  const char* name = get_str(str);
  upb_status s = UPB_STATUS_INIT;
  upb_fielddef_setname(self->fielddef, name, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Error setting FieldDef name.");
  }
  return Qnil;
}

upb_fieldtype_t ruby_to_fieldtype(VALUE type) {
  if (TYPE(type) != T_SYMBOL) {
    rb_raise(rb_eArgError, "Expected symbol for field type.");
  }

  upb_fieldtype_t upb_type = -1;

#define CONVERT(upb, ruby)                                           \
  if (SYM2ID(type) == rb_intern( # ruby )) {                         \
    upb_type = UPB_TYPE_ ## upb;                                     \
  }

  CONVERT(FLOAT, float);
  CONVERT(DOUBLE, double);
  CONVERT(BOOL, bool);
  CONVERT(STRING, string);
  CONVERT(BYTES, bytes);
  CONVERT(MESSAGE, message);
  CONVERT(ENUM, enum);
  CONVERT(INT32, int32);
  CONVERT(INT64, int64);
  CONVERT(UINT32, uint32);
  CONVERT(UINT64, uint64);

#undef CONVERT

  if (upb_type == -1) {
    rb_raise(rb_eArgError, "Unknown field type.");
  }

  return upb_type;
}

VALUE fieldtype_to_ruby(upb_fieldtype_t type) {
  switch (type) {
#define CONVERT(upb, ruby)                                           \
    case UPB_TYPE_ ## upb : return ID2SYM(rb_intern( # ruby ));
    CONVERT(FLOAT, float);
    CONVERT(DOUBLE, double);
    CONVERT(BOOL, bool);
    CONVERT(STRING, string);
    CONVERT(BYTES, bytes);
    CONVERT(MESSAGE, message);
    CONVERT(ENUM, enum);
    CONVERT(INT32, int32);
    CONVERT(INT64, int64);
    CONVERT(UINT32, uint32);
    CONVERT(UINT64, uint64);
#undef CONVERT
  }
  return Qnil;
}

VALUE FieldDef_type(VALUE _self) {
  SELF(FieldDef);
  if (!upb_fielddef_typeisset(self->fielddef)) {
    return Qnil;
  }
  return fieldtype_to_ruby(upb_fielddef_type(self->fielddef));
}

VALUE FieldDef_type_set(VALUE _self, VALUE type) {
  SELF(FieldDef);
  upb_fielddef_settype(self->fielddef, ruby_to_fieldtype(type));
  return Qnil;
}

VALUE FieldDef_label(VALUE _self) {
  SELF(FieldDef);
  switch (upb_fielddef_label(self->fielddef)) {
#define CONVERT(upb, ruby)                                           \
    case UPB_LABEL_ ## upb : return ID2SYM(rb_intern( # ruby ));

    CONVERT(OPTIONAL, optional);
    CONVERT(REQUIRED, required);
    CONVERT(REPEATED, repeated);

#undef CONVERT
  }

  return Qnil;
}

VALUE FieldDef_label_set(VALUE _self, VALUE label) {
  SELF(FieldDef);
  if (TYPE(label) != T_SYMBOL) {
    rb_raise(rb_eArgError, "Expected symbol for field label.");
  }

  upb_label_t upb_label = -1;

#define CONVERT(upb, ruby)                                           \
  if (SYM2ID(label) == rb_intern( # ruby )) {                        \
    upb_label = UPB_LABEL_ ## upb;                                   \
  }

  CONVERT(OPTIONAL, optional);
  CONVERT(REQUIRED, required);
  CONVERT(REPEATED, repeated);

#undef CONVERT

  if (upb_label == -1) {
    rb_raise(rb_eArgError, "Unknown field label.");
  }

  upb_fielddef_setlabel(self->fielddef, upb_label);

  return Qnil;
}

VALUE FieldDef_number(VALUE _self) {
  SELF(FieldDef);
  return INT2NUM(upb_fielddef_number(self->fielddef));
}

VALUE FieldDef_number_set(VALUE _self, VALUE number) {
  SELF(FieldDef);
  upb_status s = UPB_STATUS_INIT;
  upb_fielddef_setnumber(self->fielddef, NUM2INT(number), &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eArgError, "Invalid field number.");
  }
  return Qnil;
}

VALUE FieldDef_submsg_name(VALUE _self) {
  SELF(FieldDef);
  if (!upb_fielddef_hassubdef(self->fielddef)) {
    return Qnil;
  }
  const char* s = upb_fielddef_subdefname(self->fielddef);
  if (s == NULL) {
    s = "";
  }
  return rb_str_new2(s);
}

VALUE FieldDef_submsg_name_set(VALUE _self, VALUE value) {
  SELF(FieldDef);
  if (!upb_fielddef_hassubdef(self->fielddef)) {
    rb_raise(rb_eTypeError, "FieldDef does not have subdef.");
  }
  const char* str = get_str(value);
  upb_status s = UPB_STATUS_INIT;
  upb_fielddef_setsubdefname(self->fielddef, str, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Unable to set submsg_name.");
  }
  return Qnil;
}

// -----------------------------------------------------------------------------
// EnumDef.
// -----------------------------------------------------------------------------

DEFINE_CLASS(EnumDef, "Google::Protobuf::EnumDef");

void EnumDef_mark(void* _self) {
  EnumDef* self = _self;
  rb_gc_mark(self->module);
}

void EnumDef_free(void* _self) {
  EnumDef* self = _self;
  upb_enumdef_unref(self->enumdef, &self->enumdef);
  xfree(self);
}

VALUE EnumDef_alloc(VALUE klass) {
  EnumDef* self = ALLOC(EnumDef);
  self->_value = TypedData_Wrap_Struct(klass, &_EnumDef_type, self);
  self->enumdef = upb_enumdef_new(&self->enumdef);
  self->module = Qnil;
  return self->_value;
}

void EnumDef_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "EnumDef", rb_cObject);
  rb_define_alloc_func(klass, EnumDef_alloc);
  rb_define_method(klass, "name", EnumDef_name, 0);
  rb_define_method(klass, "name=", EnumDef_name_set, 1);
  rb_define_method(klass, "add_value", EnumDef_add_value, 2);
  rb_define_method(klass, "lookup_name", EnumDef_lookup_name, 1);
  rb_define_method(klass, "lookup_value", EnumDef_lookup_value, 1);
  cEnumDef = klass;
  rb_gc_register_address(&cEnumDef);
}

VALUE EnumDef_name(VALUE _self) {
  SELF(EnumDef);
  const char* s = upb_enumdef_fullname(self->enumdef);
  if (s == NULL) {
    s = "";
  }
  return rb_str_new2(s);
}

VALUE EnumDef_name_set(VALUE _self, VALUE str) {
  SELF(EnumDef);
  const char* name = get_str(str);
  upb_status s = UPB_STATUS_INIT;
  upb_enumdef_setfullname(self->enumdef, name, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Error setting EnumDef name.");
  }
  return Qnil;
}

VALUE EnumDef_add_value(VALUE _self, VALUE name, VALUE number) {
  SELF(EnumDef);
  const char* name_str = rb_id2name(SYM2ID(name));
  int32_t val = NUM2INT(number);
  upb_status s = UPB_STATUS_INIT;
  upb_enumdef_addval(self->enumdef, name_str, val, &s);
  if (!upb_ok(&s)) {
    rb_raise(rb_eRuntimeError, "Error adding value to enum.");
  }
  return Qnil;
}

VALUE EnumDef_lookup_name(VALUE _self, VALUE name) {
  SELF(EnumDef);
  const char* name_str= rb_id2name(SYM2ID(name));
  int32_t val = 0;
  if (upb_enumdef_ntoi(self->enumdef, name_str, &val)) {
    return INT2NUM(val);
  } else {
    return Qnil;
  }
}

VALUE EnumDef_lookup_value(VALUE _self, VALUE number) {
  SELF(EnumDef);
  int32_t val = NUM2INT(number);
  const char* name = upb_enumdef_iton(self->enumdef, val);
  if (name != NULL) {
    return ID2SYM(rb_intern(name));
  } else {
    return Qnil;
  }
}

// -----------------------------------------------------------------------------
// MessageBuilderContext.
// -----------------------------------------------------------------------------

DEFINE_CLASS(MessageBuilderContext,
    "Google::Protobuf::Internal::MessageBuilderContext");

void MessageBuilderContext_mark(void* _self) {
  MessageBuilderContext* self = _self;
  rb_gc_mark(self->msgdef);
}

void MessageBuilderContext_free(void* _self) {
  MessageBuilderContext* self = _self;
  xfree(self);
}

VALUE MessageBuilderContext_alloc(VALUE klass) {
  MessageBuilderContext* self = ALLOC(MessageBuilderContext);
  self->_value = TypedData_Wrap_Struct(
      klass, &_MessageBuilderContext_type, self);
  self->msgdef = Qnil;
  return self->_value;
}

void MessageBuilderContext_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "MessageBuilderContext", rb_cObject);
  rb_define_alloc_func(klass, MessageBuilderContext_alloc);
  rb_define_method(klass, "initialize",
                   MessageBuilderContext_initialize, 1);
  rb_define_method(klass, "optional", MessageBuilderContext_optional, -1);
  rb_define_method(klass, "required", MessageBuilderContext_required, -1);
  rb_define_method(klass, "repeated", MessageBuilderContext_repeated, -1);
  cMessageBuilderContext = klass;
  rb_gc_register_address(&cMessageBuilderContext);
}

VALUE MessageBuilderContext_initialize(VALUE _self, VALUE msgdef) {
  SELF(MessageBuilderContext);
  self->msgdef = msgdef;
  return Qnil;
}

static VALUE msgdef_add_field(VALUE msgdef,
                              const char* label, VALUE name,
                              VALUE type, VALUE number,
                              VALUE type_class) {
  VALUE fielddef = rb_class_new_instance(0, NULL, cFieldDef);
  VALUE name_str = rb_str_new2(rb_id2name(SYM2ID(name)));

  rb_funcall(fielddef, rb_intern("label="), 1, ID2SYM(rb_intern(label)));
  rb_funcall(fielddef, rb_intern("name="), 1, name_str);
  rb_funcall(fielddef, rb_intern("type="), 1, type);
  rb_funcall(fielddef, rb_intern("number="), 1, number);

  if (type_class != Qnil) {
    if (TYPE(type_class) != T_STRING) {
      rb_raise(rb_eArgError, "Expected string for type class");
    }
    // Make it an absolute type name by prepending a dot.
    type_class = rb_str_append(rb_str_new2("."), type_class);
    rb_funcall(fielddef, rb_intern("submsg_name="), 1, type_class);
  }

  rb_funcall(msgdef, rb_intern("add_field"), 1, fielddef);
  return fielddef;
}

VALUE MessageBuilderContext_optional(int argc, VALUE* argv, VALUE _self) {
  SELF(MessageBuilderContext);

  if (argc < 3) {
    rb_raise(rb_eArgError, "Expected at least 3 arguments.");
  }
  VALUE name = argv[0];
  VALUE type = argv[1];
  VALUE number = argv[2];
  VALUE type_class = (argc > 3) ? argv[3] : Qnil;

  return msgdef_add_field(self->msgdef, "optional", name, type, number, type_class);
}

VALUE MessageBuilderContext_required(int argc, VALUE* argv, VALUE _self) {
  SELF(MessageBuilderContext);

  if (argc < 3) {
    rb_raise(rb_eArgError, "Expected at least 3 arguments.");
  }
  VALUE name = argv[0];
  VALUE type = argv[1];
  VALUE number = argv[2];
  VALUE type_class = (argc > 3) ? argv[3] : Qnil;

  return msgdef_add_field(self->msgdef, "required", name, type, number, type_class);
}

VALUE MessageBuilderContext_repeated(int argc, VALUE* argv, VALUE _self) {
  SELF(MessageBuilderContext);

  if (argc < 3) {
    rb_raise(rb_eArgError, "Expected at least 3 arguments.");
  }
  VALUE name = argv[0];
  VALUE type = argv[1];
  VALUE number = argv[2];
  VALUE type_class = (argc > 3) ? argv[3] : Qnil;

  return msgdef_add_field(self->msgdef, "repeated", name, type, number, type_class);
}

// -----------------------------------------------------------------------------
// EnumBuilderContext.
// -----------------------------------------------------------------------------

DEFINE_CLASS(EnumBuilderContext,
    "Google::Protobuf::Internal::EnumBuilderContext");

void EnumBuilderContext_mark(void* _self) {
  EnumBuilderContext* self = _self;
  rb_gc_mark(self->enumdef);
}

void EnumBuilderContext_free(void* _self) {
  EnumBuilderContext* self = _self;
  xfree(self);
}

VALUE EnumBuilderContext_alloc(VALUE klass) {
  EnumBuilderContext* self = ALLOC(EnumBuilderContext);
  self->_value = TypedData_Wrap_Struct(
      klass, &_EnumBuilderContext_type, self);
  self->enumdef = Qnil;
  return self->_value;
}

void EnumBuilderContext_register(VALUE module) {
  VALUE klass = rb_define_class_under(
      module, "EnumBuilderContext", rb_cObject);
  rb_define_alloc_func(klass, EnumBuilderContext_alloc);
  rb_define_method(klass, "initialize",
                   EnumBuilderContext_initialize, 1);
  rb_define_method(klass, "value", EnumBuilderContext_value, 2);
  cEnumBuilderContext = klass;
  rb_gc_register_address(&cEnumBuilderContext);
}

VALUE EnumBuilderContext_initialize(VALUE _self, VALUE enumdef) {
  SELF(EnumBuilderContext);
  self->enumdef = enumdef;
  return Qnil;
}

static VALUE enumdef_add_value(VALUE enumdef,
                               VALUE name, VALUE number) {
  rb_funcall(enumdef, rb_intern("add_value"), 2, name, number);
  return Qnil;
}

VALUE EnumBuilderContext_value(VALUE _self, VALUE name, VALUE number) {
  SELF(EnumBuilderContext);
  return enumdef_add_value(self->enumdef, name, number);
}

// -----------------------------------------------------------------------------
// Builder.
// -----------------------------------------------------------------------------

DEFINE_CLASS(Builder, "Google::Protobuf::Internal::Builder");

void Builder_mark(void* _self) {
  Builder* self = _self;
  rb_gc_mark(self->pending_list);
}

void Builder_free(void* _self) {
  Builder* self = _self;
  xfree(self);
}

VALUE Builder_alloc(VALUE klass) {
  Builder* self = ALLOC(Builder);
  self->_value = TypedData_Wrap_Struct(
      klass, &_Builder_type, self);
  self->pending_list = rb_ary_new();
  return self->_value;
}

void Builder_register(VALUE module) {
  VALUE klass = rb_define_class_under(module, "Builder", rb_cObject);
  rb_define_alloc_func(klass, Builder_alloc);
  rb_define_method(klass, "add_message", Builder_add_message, 1);
  rb_define_method(klass, "add_enum", Builder_add_enum, 1);
  rb_define_method(klass, "finalize_to_symtab", Builder_finalize_to_symtab, 1);
  cBuilder = klass;
  rb_gc_register_address(&cBuilder);
}

VALUE Builder_add_message(VALUE _self, VALUE name) {
  SELF(Builder);
  VALUE msgdef = rb_class_new_instance(0, NULL, cMessageDef);
  VALUE ctx = rb_class_new_instance(1, &msgdef, cMessageBuilderContext);
  VALUE block = rb_block_proc();
  rb_funcall(msgdef, rb_intern("name="), 1, name);
  rb_funcall_with_block(ctx, rb_intern("instance_eval"), 0, NULL, block);
  rb_ary_push(self->pending_list, msgdef);
  return Qnil;
}

VALUE Builder_add_enum(VALUE _self, VALUE name) {
  SELF(Builder);
  VALUE enumdef = rb_class_new_instance(0, NULL, cEnumDef);
  VALUE ctx = rb_class_new_instance(1, &enumdef, cEnumBuilderContext);
  VALUE block = rb_block_proc();
  rb_funcall(enumdef, rb_intern("name="), 1, name);
  rb_funcall_with_block(ctx, rb_intern("instance_eval"), 0, NULL, block);
  rb_ary_push(self->pending_list, enumdef);
  return Qnil;
}

static void validate_msgdef(const upb_msgdef* msgdef) {
  // Verify that no required fields exist. proto3 does not support these.
  upb_msg_iter it;
  for (upb_msg_begin(&it, msgdef); !upb_msg_done(&it); upb_msg_next(&it)) {
    const upb_fielddef* field = upb_msg_iter_field(&it);
    if (upb_fielddef_label(field) == UPB_LABEL_REQUIRED) {
      rb_raise(rb_eTypeError, "Required fields are unsupported in proto3.");
    }
  }
}

static void validate_enumdef(const upb_enumdef* enumdef) {
  // Verify that an entry exists with integer value 0. (This is the default
  // value.)
  const char* lookup = upb_enumdef_iton(enumdef, 0);
  if (lookup == NULL) {
    rb_raise(rb_eTypeError,
             "Enum definition does not contain a value for '0'.");
  }
}

VALUE Builder_finalize_to_symtab(VALUE _self, VALUE symtab_rb) {
  SELF(Builder);

  SymbolTable* symtab = ruby_to_SymbolTable(symtab_rb);

  upb_def** defs = ALLOC_N(upb_def*,
                           RARRAY_LEN(self->pending_list));
  for (int i = 0; i < RARRAY_LEN(self->pending_list); i++) {
    VALUE def_rb = rb_ary_entry(self->pending_list, i);
    if (CLASS_OF(def_rb) == cMessageDef) {
      defs[i] = (upb_def*)ruby_to_MessageDef(def_rb)->msgdef;
      validate_msgdef((const upb_msgdef*)defs[i]);
    } else if (CLASS_OF(def_rb) == cEnumDef) {
      defs[i] = (upb_def*)ruby_to_EnumDef(def_rb)->enumdef;
      validate_enumdef((const upb_enumdef*)defs[i]);
    }
  }

  upb_status s = UPB_STATUS_INIT;
  upb_symtab_add(symtab->symtab, (upb_def**)defs,
                 RARRAY_LEN(self->pending_list), NULL, &s);
  if (!upb_ok(&s)) {
    xfree(defs);
    rb_raise(rb_eRuntimeError, "Unable to add defs to SymbolTable: %s",
             upb_status_errmsg(&s));
  }

  for (int i = 0; i < RARRAY_LEN(self->pending_list); i++) {
    VALUE def_rb = rb_ary_entry(self->pending_list, i);
    add_def_obj(defs[i], def_rb);
  }

  xfree(defs);
  self->pending_list = rb_ary_new();
  return Qnil;
}
