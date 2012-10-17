require 'spec_helper'

describe 'The TLearnExt C Extension' do

  def tlearn_extension
    File.exists?(File.dirname(__FILE__) + '/../lib/tlearn.so') ? 'tlearn.so' : 'tlearn.bundle'
  end

  before(:all) do
    require tlearn_extension
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