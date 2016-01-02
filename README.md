# TLearn Ruby

[![Build Status](https://secure.travis-ci.org/josephwilk/tlearn-rb.png)](http://travis-ci.org/josephwilk/tlearn-rb)

Recurrent Neural Network library for Ruby which uses the tlearn neural network simulator(http://crl.ucsd.edu/innate/tlearn.html)

To see an example run:

```
rake example:train
rake example:fitness
```

# Seriously? Just use FANN!

TLearn supports recurrent networks (http://en.wikipedia.org/wiki/Recurrent_neural_network), FANN does not. Recurrent networks maintain state, enabling the context of previous inputs to effect further outputs. 

While there have been attempts to add recurrent networks to FANN (http://leenissen.dk/fann/forum/viewtopic.php?t=47) these are still sitting on a dead branch that was never merged into master.

# Installing TLearn

`gem install tlearn`

#Usage


```ruby
require 'tlearn'

tlearn = TLearn::Run.new(:number_of_nodes => 86,
                         :output_nodes    => 41..46,
                         :linear          => 47..86,
                         :weight_limit    => 1.00,
                         :connections     => [{1..81   => 0},
                                              {1..40   => :i1..:i77},
                                              {41..46  => 1..40},
                                              {1..40   => 47..86},
                                              {47..86  => [1..40, {:max => 1.0, :min => 1.0}, :fixed, :one_to_one]}])
                 
  
training_data = [{[0] * 77 => [1, 0, 0, 0, 0, 0]}],
                [{[1] * 77 => [0, 0, 0, 0, 0, 1]}]
  
tlearn.train(training_data, sweeps = 200)

tlearn.fitness([0] * 77, sweeps = 200)
# => ["0.016", "0.013", "0.022", "0.020", "0.463", "0.467"]
```

You will often seperate the training and fitness process. By specifying a working directory when training you can reuse the training data for later
fitness evaulations.

```ruby
tlearn.train(training_data, sweeps = 200, working_dir='/training_session/')
```

```ruby
tlearn.fitness([0] * 77, sweeps = 200, working_dir = '/training_session/')
# => ["0.016", "0.013", "0.022", "0.020", "0.463", "0.467"]
```

Configuring TLearn (What the heck does all that config mean?)
=========

Yes, its complicated configuring this thing. Lets work through the different configuration options:

    #Total number of nodes in the network (not counting input).
    :number_of_nodes => 86

    #The nodes that are used for output.
    :'output_nodes' => 41..46

    nodes 1-10 are linear. Linear nodes ouput the inner-product of the input and weight vectors
    :linear_nodes => 1..10

    #nodes 1-10 are bipolar. Bipolar nodes output ranges continuously from -1 to +1.
    :bipolar_nodes => 1..10

    #weights between nodes will not exceed 1.00
    :weight_limit => 1.00

<h4>Connections</h4>
Here we specify how all of our nodes are connected, the architecture of the neural network. 

We use ranges to specify connections between nodes:

    1..3 => 4..6

Indicates connections:

    node 1 <- node 4 
    node 1 <- node 5
    node 1 <- node 6

    node 2 <- node 4
    node 2 <- node 5
    node 2 <- node 6

    node 3 <- node 4
    node 3 <- node 5
    node 3 <- node 6

Note that the nodes specified first (1..3) are the destination nodes, the second nodes (4..6) are the source nodes. The sources nodes feed into the destination nodes.

<p>Make sure you feed the input nodes into the network.</p>

    #1-6 nodes are fed input nodes 1-10.
    1..6 => i1..i10

<p>We can also add constraints to the different connections:</p>
    #nodes 1-6 connections with nodes 7-9 will have weights never less than 1 or greater than 10.
    1..6 => [7..9, {:min => 1, :max => 10}]

    #nodes 1-6 are fed from node 0 (node 0 is always the bias node).
    1..6 => 0

    #nodes 1-6 connections with nodes 7-10 are fixed at initiation values and will not change throughout learning.
    1..6 => [7..10, :fixed]

    #nodes 1-6 connections with nodes 7-10 are fixed at 2 and will not change throughout learning.
    1..6 => [7..9, {:min => 2, :max => 2}]

    #1-6 nodes connections with nodes from 7-9 are fixed at weight 1. 
    1..3 => [7..9, {:min => 1.0, :max => 1.0}, :fixed, :'one_to_one']

one_to_one changes the way connections are mapped. Instead of one node mapping to every other node we have a 1-1 mapping between nodes:

For example:

    node 1 -> node 7
    mode 2 -> node 8
    node 3 -> node 9

There is also the TLearn manual if you want read more:

http://blog.josephwilk.net/uploads/tlearn.pdf


#C TLearn Library

The Original TLearn C library was written by:
* Kim Plunkett
* Jeffrey L. Elman

#Contributors


* Joseph Wilk [blog](http://blog.josephwilk.net) | [twitter](http://twitter.com/josephwilk)

#License

(The MIT License)

Copyright (c) 2012-2016 Joseph Wilk

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
