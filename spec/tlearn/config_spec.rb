require 'spec_helper'
require 'fakefs/spec_helpers'

module TLearn
  describe Config do
    include FakeFS::SpecHelpers
    
    before(:each) do
      @config = Config.new(settings)
      FileUtils.mkdir_p @config.file_root
    end
    
    let(:settings){
      {:nodes       => {:number_of_nodes => 9999,
                         :output_nodes    => 41..46},
       :connections => [{1..81   => 0},
                         {1..40   => 'i1-i77'},
                         {1..40   => '47-86'},
                         {47..86  => '1-40 = 1. & 1. fixed one-to-one'}],
       :special => {:weight_limit => 1.0}
      }
    }
    
    let(:training_data){
      TrainingData.new([[{[1] * 77 => [0, 0, 0, 0, 0, 1]}],[{[0] * 77 => [1, 0, 0, 0, 0, 0]}]])
    }

    def config_file
      File.read("#{@config.file_root}.cf")
    end
    
    describe "generating *.cf file" do
      before(:each) do
        @config.setup_config(training_data)
      end

      it "should contain weight limit" do
        config_file.should include "weight_limit = 1.0"
      end

      it "should contain the number of nodes" do
        config_file.should include "9999"
      end

      it "should contain the output nodes" do
        config_file.should include "41-46"
      end

      it "should convert all config ranges to strings" do 
        config_file.should include "1-81"
      end
      
      it "should allow multiple values for a single range" do
        config_file.scan(/^1-40/).should == ["1-40", "1-40"]
      end
    end
    
  end
end