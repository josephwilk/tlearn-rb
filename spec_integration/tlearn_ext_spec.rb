require 'spec_helper'

describe 'The TLearnExt C Extension' do

  it "should not segfault" do
    require 'tlearn.so'

    TLearnExt.run({}) rescue{}
  end
  
end