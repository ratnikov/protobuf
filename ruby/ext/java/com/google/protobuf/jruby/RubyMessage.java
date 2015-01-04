package com.google.protobuf.jruby;

import com.google.protobuf.*;
import com.google.protobuf.Descriptors.*;
import org.jruby.*;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.*;
import org.jruby.runtime.builtin.IRubyObject;
import org.jruby.internal.runtime.methods.*;
import org.jruby.util.ByteList;

public class RubyMessage extends RubyObject {

  public static RubyClass newClass(ThreadContext ctx, Descriptor descriptor) {
    RubyClass messageClass = RubyClass.newClass(ctx.runtime, ctx.runtime.getObject());
    messageClass.setAllocator(new Allocator(descriptor));

    for (FieldDescriptor field : descriptor.getFields()) {
      messageClass.addMethod(field.getName(), new FieldSetterMethod(messageClass, field));
    }

    return messageClass;
  }

  private final DynamicMessage.Builder builder;

  public RubyMessage(Ruby runtime, RubyClass metaClass, Descriptor descriptor) {
    super(runtime, metaClass);

    this.builder = DynamicMessage.newBuilder(descriptor);
  }

  IRubyObject getFieldValue(ThreadContext ctx, FieldDescriptor field) {
    Ruby runtime = ctx.runtime;
    Object rawValue = builder.getField(field);
    
    switch (field.getJavaType()) {
      case INT:
	return runtime.newFixnum((Integer) rawValue);
      case LONG:
	return runtime.newFixnum((Long) rawValue);
      case FLOAT:
	return runtime.newFloat((Float) rawValue);
      case DOUBLE:
	return runtime.newFloat((Double) rawValue);
      case BOOLEAN:
	return runtime.newBoolean((Boolean) rawValue);
      case STRING:
	return runtime.newString((String) rawValue);
      case BYTE_STRING:
	ByteString byteString = (ByteString) rawValue;
	return runtime.newString(new ByteList(byteString.toByteArray(), false));
      case MESSAGE:
	DynamicMessage message = (DynamicMessage) rawValue;
	if (message.getSerializedSize() > 0) {
	  throw runtime.newArgumentError("Not supported yet");
	} else {
	  return runtime.getNil();
	}
      case ENUM:
	EnumValueDescriptor enumValue = (EnumValueDescriptor) rawValue;
	return runtime.newSymbol(enumValue.getName());
      default:
	System.out.println("raw: " + rawValue.getClass());
	throw runtime.newArgumentError("boo");
    }
  }

  private static class Allocator implements ObjectAllocator {
    private final Descriptor descriptor;

    public Allocator(Descriptor descriptor) {
      this.descriptor = descriptor;
    }

    public IRubyObject allocate(Ruby runtime, RubyClass metaClass) {
      return new RubyMessage(runtime, metaClass, descriptor);
    }
  }

  private static class FieldSetterMethod extends JavaMethod.JavaMethodZero {
    private final FieldDescriptor field;

    public FieldSetterMethod(RubyClass metaClass, FieldDescriptor field) {
      super(metaClass, Visibility.PUBLIC);
      this.field = field;
    }

    @Override
    public IRubyObject call(ThreadContext ctx, IRubyObject self, RubyModule ignored, String ignored2) {
      return ((RubyMessage) self).getFieldValue(ctx, field);
    }
  }
}
