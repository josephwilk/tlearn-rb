require 'spec_helper'

describe 'TLearnExt' do
  
  it "should not segfault" do
    require 'tlearn.bundle'
    TLearnExt.run({}) rescue{}
  end
  
end