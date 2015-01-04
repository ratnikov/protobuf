package com.google.protobuf.jruby;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;
import com.google.protobuf.Descriptors.FileDescriptor;
import org.jruby.*;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.*;
import org.jruby.internal.runtime.methods.JavaMethod;

public class RubyDescriptor extends RubyObject {
  public static RubyClass createDescriptorClass(Ruby runtime) {
    RubyModule parentMod = (RubyModule) runtime.getModule("Google")
      .getConstantAt("Protobuf");

    RubyClass descriptorClass = runtime.defineClassUnder("Descriptor", runtime.getObject(), DESCRIPTOR_ALLOCATOR, parentMod);
    descriptorClass.defineAnnotatedMethods(RubyDescriptor.class);

    return descriptorClass;
  }

  private static ObjectAllocator DESCRIPTOR_ALLOCATOR = new ObjectAllocator() {
    @Override
    public IRubyObject allocate(Ruby runtime, RubyClass metaClass) {
      return new RubyDescriptor(runtime, metaClass);
    }
  };

  private FileDescriptor fileDescriptor = null;

  public RubyDescriptor(Ruby runtime, RubyClass metaClass) {
    super(runtime, metaClass);
  }

  @JRubyMethod
  public void initialize(ThreadContext ctx, IRubyObject name) {
    try {
      this.fileDescriptor = (FileDescriptor) ctx.runtime.getJRubyClassLoader().loadClass(name.asJavaString()).getMethod("getDescriptor").invoke(null);
    } catch (Exception e) { // TODO be more specific about exceptions
      e.printStackTrace();
      throw ctx.runtime.newArgumentError("Descriptor " + name.asJavaString() + " does not exist.");
    }
  }

  @JRubyMethod
  public IRubyObject lookup(ThreadContext ctx, IRubyObject name) {
    Descriptor messageDescriptor = fileDescriptor.findMessageTypeByName(name.asJavaString());
    if (messageDescriptor != null) {
      return RubyMessage.newClass(ctx, messageDescriptor);
    } else {
      return ctx.runtime.getNil();
    }
  }
}
