require "bundler"
Bundler.setup
Bundler::GemHelper.install_tasks

require 'rspec/core/rake_task'
require 'rake/extensiontask'

Rake::ExtensionTask.new('tlearn')

RSpec::Core::RakeTask.new(:spec)
RSpec::Core::RakeTask.new(:spec_integration) do |spec|
  spec.pattern = 'spec_integration/**/*_spec.rb'
end

def tlearn_extension_present?
  File.exists?(File.dirname(__FILE__) + '/tlearn.so') || File.exists?(File.dirname(__FILE__) + '/tlearn.bundle')
end

if tlearn_extension_present?

  namespace :example do
        require File.dirname(__FILE__) + '/lib/tlearn'

    neural_network_config =   {:nodes       => {:number_of_nodes => 86,
                                                :output_nodes => '41-46'},
                               :connections => [{1..81   => '0'},
                                                {1..40   => 'i1-i77'},
                                                {41..46  => '1-40'},
                                                {1..40   =>  '47-86'},
                                                {47..86  => ' 1-40 = 1. & 1. fixed one-to-one'}],
                               :special      => {:linear => '47-86',
                               :weight_limit => 1.00,
                               :selected     => '1-86'}}

    SWEEPS = 1000

    tlearn = TLearn::Run.new(neural_network_config)
                                          
    desc "Start a training session"
    task :train do
      training_data = [{[0] * 77 => [1, 0, 0, 0, 0, 0]}],
                      [{[1] * 77 => [0, 0, 0, 0, 0, 1]}]
  
      tlearn.train(training_data, sweeps = SWEEPS)
    end

    desc "Use trained network to evaluate an input"
    task :fitness => [:train] do
      test_subject_1 = [0] * 77
      test_subject_2 = [1] * 77
  
      rating_1 = tlearn.fitness(test_subject_1, sweeps = SWEEPS)
      rating_2 = tlearn.fitness(test_subject_2, sweeps = SWEEPS)

      [[rating_1, test_subject_1], [rating_2, test_subject_2]].each do |ratings, subject|
        rank = ratings.rindex(ratings.max) + 1
        puts "rank: #{rank} => #{subject}"
        p ratings, ""
      end
    end
  end

end

task :default => [:compile, :spec, :spec_integration]