require 'spec_helper'

module TLearn
  describe TrainingData do
    let(:all_zeros){ [0] * 77 }
    let(:all_ones) { [1] * 77 }
    
    describe '.reset_points' do
      it "should create the reset points from the training data" do
        training_data =  [
                           [{all_zeros => [1, 0, 0, 0, 0, 0]}, {all_zeros => [1, 0, 0, 0, 0, 0]}],
                           [{all_ones  => [0, 0, 0, 0, 0, 1]}]
                         ]
            
        training_data = TrainingData.new(training_data)
  
        training_data.reset_points.should == [2, 1]
      end
    
      it "should caculate the training data length" do
      
      end
    end
    
    describe '#no_of_data_values' do
      it "should return the total number of inputs" do
        training_data =  [
                           [{all_zeros => [1, 0, 0, 0, 0, 0]}, {all_zeros => [1, 0, 0, 0, 0, 0]}],
                           [{all_ones  => [0, 0, 0, 0, 0, 1]}]
                         ]
            
        training_data = TrainingData.new(training_data)
  
        training_data.no_of_data_values.should == 3
      end
    end
    
    describe '.no_of_inputs' do
      it "should return the size of all the training data" do
        training_data =  [
                           [{all_zeros => [1, 0, 0, 0, 0, 0]}, {all_zeros => [1, 0, 0, 0, 0, 0]}],
                           [{all_ones  => [0, 0, 0, 0, 0, 1]}]
                         ]
            
        training_data = TrainingData.new(training_data)
  
        training_data.no_of_inputs.should == 77
      end
    end

    describe '.no_of_ouputs' do
      it "should return the size of all the training data" do
        training_data =  [
                           [{all_zeros => [1, 0, 0, 0, 0, 0]}, {all_zeros => [1, 0, 0, 0, 0, 0]}],
                           [{all_ones  => [0, 0, 0, 0, 0, 1]}]
                         ]
            
        training_data = TrainingData.new(training_data)
  
        training_data.no_of_outputs.should == 6
      end
    end
    
  end
end