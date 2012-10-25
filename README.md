TLearn Ruby
=========

[![Build Status](https://secure.travis-ci.org/josephwilk/tlearn-rb.png)](http://travis-ci.org/josephwilk/tlearn-rb)

Ruby helping make tlearn neural network simulator sane(ish) (http://crl.ucsd.edu/innate/tlearn.html)

To see an example run:

<pre><code>rake example:train
rake example:fitness
</code></pre>

Seriously? Just use FANN!
=========

TLearn supports recurrent networks (http://en.wikipedia.org/wiki/Recurrent_neural_network), FANN does not. Recurrent networks maintain state, enabling the context of previous inputs to effect further outputs. 

While there have been attempts to add recurrent networks to FANN (http://leenissen.dk/fann/forum/viewtopic.php?t=47) these are still sitting on a dead branch that was never merged into master.

Installing TLearn
=========

<pre><code>gem install tlearn</code></pre>

Usage
=========

```ruby
require 'tlearn'

tlearn = TLearn.new(:number_of_nodes => 86,
                    :output_nodes    => 41..46,
                    :linear          => 47..86,
                    :weight_limit    => 1.00,
                    :selected        => 1..86,
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

<pre>#Total number of nodes in the network (not counting input).
:number_of_nodes => 86</pre>

<pre>#The nodes that are used for output.
:'output_nodes' => 41..46</pre>

<pre>nodes 1-10 are linear.
:linear_nodes => 1..10</pre>

<pre>#nodes 1-10 are bipolar.
:bipolar_nodes => 1..10</pre>

<h4>Connections(how nodes connect to each other):</h4>

<pre>#nodes 1-6 connections will never be less than 1 or greater than 10.
1..6 => {:min => 1, :max => 10}</pre>

<pre>#nodes 1-6 are fed from node 0 (node 0 is always the bias node).
1..6 => 0 </pre> 

<pre>#nodes 1-6 connection weights are fixed at 2 and will not change throughout learning.
1..6 => {:min => 2, :max => 2] </pre>

<pre>#nodes 1-6 connections with nodes 7-10 are fixed at initiation values and will not change throughout learning.
1..6 => [7..10, :fixed]</pre>

<pre>#1-6 nodes connections with nodes from 7-9 are fixed at weight 1. 
1..6 => [7..9, {:min => 1.0, :max => 1.0}, :fixed, :'one_to_one'] </pre>

one_to_one means:

* Node 1 is fed from node 7,
* Node 2 is fed from node 8
* Node 3 is feed from node 9.

The normal case (without one-to-one) is:

* Node 1 is fed from 7-9
* Node 2 is fed from 7-9
* Node 3 is fed from 7-9
 
<pre>#1-6 nodes are fed input nodes 1-10.
1..6 => i1..i10</pre> 


There is also the TLearn manual if you want read more:

http://blog.josephwilk.net/wp-content/uploads/2012/10/tlearn.pdf


C TLearn Library
=========

The Original TLearn C library was written by:
* Kim Plunkett
* Jeffrey L. Elman

Contributors
=========

* Joseph Wilk [blog](http://blog.josephwilk.net) | [twitter](http://twitter.com/josephwilk)

License
=========

(The MIT License)

Copyright (c) 2012 Joseph Wilk

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