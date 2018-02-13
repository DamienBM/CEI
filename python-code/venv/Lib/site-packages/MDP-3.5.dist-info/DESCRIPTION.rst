**The Modular toolkit for Data Processing (MDP)** package is a library
of widely used data processing algorithms, and the possibility to
combine them together to form pipelines for building more complex
data processing software.

MDP has been designed to be used as-is and as a framework for
scientific data processing development.

>From the user's perspective, MDP consists of a collection of *units*,
which process data. For example, these include algorithms for
supervised and unsupervised learning, principal and independent
components analysis and classification.

These units can be chained into data processing flows, to create
pipelines as well as more complex feed-forward network
architectures. Given a set of input data, MDP takes care of training
and executing all nodes in the network in the correct order and
passing intermediate data between the nodes. This allows the user to
specify complex algorithms as a series of simpler data processing
steps.

The number of available algorithms is steadily increasing and includes
signal processing methods (Principal Component Analysis, Independent
Component Analysis, Slow Feature Analysis), manifold learning methods
([Hessian] Locally Linear Embedding), several classifiers,
probabilistic methods (Factor Analysis, RBM), data pre-processing
methods, and many others.

Particular care has been taken to make computations efficient in terms
of speed and memory. To reduce the memory footprint, it is possible to
perform learning using batches of data. For large data-sets, it is
also possible to specify that MDP should use single precision floating
point numbers rather than double precision ones. Finally, calculations
can be parallelised using the ``parallel`` subpackage, which offers a
parallel implementation of the basic nodes and flows.

>From the developer's perspective, MDP is a framework that makes the
implementation of new supervised and unsupervised learning algorithms
easy and straightforward. The basic class, ``Node``, takes care of tedious
tasks like numerical type and dimensionality checking, leaving the
developer free to concentrate on the implementation of the learning
and execution phases. Because of the common interface, the node then
automatically integrates with the rest of the library and can be used
in a network together with other nodes.

A node can have multiple training phases and even an undetermined
number of phases. Multiple training phases mean that the training data
is presented multiple times to the same node. This allows the
implementation of algorithms that need to collect some statistics on
the whole input before proceeding with the actual training, and others
that need to iterate over a training phase until a convergence
criterion is satisfied. It is possible to train each phase using
chunks of input data if the chunks are given as an iterable. Moreover,
crash recovery can be optionally enabled, which will save the state of
the flow in case of a failure for later inspection.

MDP is distributed under the open source BSD license. It has been
written in the context of theoretical research in neuroscience, but it
has been designed to be helpful in any context where trainable data
processing algorithms are used. Its simplicity on the user's side, the
variety of readily available algorithms, and the reusability of the
implemented nodes also make it a useful educational tool.

http://mdp-toolkit.sourceforge.net

