#!/usr/bin/ruby

require 'protobuf'
require 'test/unit'

symtab = Google::Protobuf::SymbolTable.global_symtab
symtab.build do
  add_message "TestMessage" do
    optional :a,  :int32,        1
    repeated :b,  :message,      2, "M"
  end
  add_message "M" do
    optional :foo, :string, 1
  end
end

TestMessage = symtab.get_class("TestMessage")
M = symtab.get_class("M")

def get_msg
  TestMessage.new(:a => 1000,
                  :b => [M.new(:foo => "hello"),
                         M.new(:foo => "world")])
end

class StressTest < Test::Unit::TestCase
  def test_stress
    m = get_msg
    data = TestMessage.encode(m)
    100_000.times do
      mnew = TestMessage.decode(data)
      mnew = mnew.dup
      assert mnew.inspect == m.inspect
      assert TestMessage.encode(mnew) == data
    end
  end
end
