require 'spec_helper'

describe 'The TLearnExt C Extension' do

  def tlearn_extension
    if File.exists?(File.dirname(__FILE__) + '/../lib/tlearn.so')
      'tlearn.so'
    else
      'tlearn.bundle'
    end
  end
  
  it "should not segfault" do
    require tlearn_extension

    TLearnExt.run({}) rescue{}
  end
  
end