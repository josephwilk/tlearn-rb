require 'spec_helper'

describe 'The TLearnExt C Extension' do

  before(:all) do
    require 'tlearn.so'
    require 'tlearn.bundle'
  end

  it "should not segfault" do
    TLearnExt.train({})   rescue{}
    TLearnExt.fitness({}) rescue{}
  end

  it "should support training" do
    TLearnExt.train({'sweeps' => '1', 'file_root' => 'test'})
  end
  
  it "should support fitness" do
    TLearnExt.fitness({'sweeps' => '1', 'file_root' => 'test'})
  end
  
end