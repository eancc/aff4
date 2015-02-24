---
layout: docs
title: AFF4 Introduction and basic concepts.
order: 1
---

## History

AFF4 is an evolution in forensic imaging technology. It is instructive to
examine where forensic images came from, historically, in order to understand
what AFF4 is attempting to solve.

### Raw images.

The raw image (or sometimes named the *dd* image) is the oldest forms of
forensic images. It was historically obtained by running the dd command to copy
raw bytes from a disk into an image file. The dd image has a simple format -
each byte in the image corresponds to a precise byte in the source media.

Despite their extreme simplicity, a raw image has several shortcomings:

1. It can contain no compression - the size of the image is the same as the size
of the acquired disk.

2. It is typically not possible to represent a sparse source media - in some
applications (e.g. a memory image), not all source regions are readable. Raw
images must pad these regions with something to maintain alignment (typically
unavailable regions are padded with zeros).

3. No metadata is kept with the image. metadata must be kept externally and
manually associated with the image.

Despite these limitations, raw images are still very popular since they are very
simple - there are a number of hardware based imagers on the marker which can
produce raw images making it fairly convenient.

### AFF (up to version 3).

AFF, the predecessor to AFF4 make significant improvements to the state of the
art in acquisitions. The AFF format offered compression and started storing
metadata within the image.

Other similar commercial solutions also offered similar features (such as the
EWF format) - more diverse metadata was added to these file formats and features
such as encryption and digital signatures were also offered.

Each forensic image format introduced a unique file format. This required a
unique tool to read it and often require reverse engineering work (for
proprietary tools).


## The information model.

Rather than treat metadata as an after thought, the AFF4 format uses metadata as
its central abstraction - everything is built around the metadata. In this
regard AFF4 is more than simply a file format - it is more akin to a complete
evidence management system.

In AFF4 terminology, all known information about the world is stored in an
*Information Model*. The AFF4 information model is centered around the concept
of AFF4 objects.

The following are some basic assumptions about the AFF4 information model:

1. AFF4 Objects are simply entities about which statements are made. AFF4
objects have a globally unique name (called a URN). To make URNs unique we often
use a GUID to generate a unique part of the name.

2. AFF4 uses RDF to model the statement about the object. An RDF statement is
simply a tuple of (subject, predicate, value), where subject is the URN we talk
about and predicate is a verb from a known lexicon.

3. AFF4 statements are stored in a *Resolver*. The resolver is a central point
which manages the AFF4 information model. Note that typically the resolver will
need to be populated with metadata before any AFF4 objects can be instantiated.

For example, consider the following snippet of RDF encoded in the *Turtle*
encoding:

```turtle
<aff4://21803da1-a4e4-4cf7-b866-f4f94c6fb041/bin/zcmp>
    aff4:chunk_size 32768 ;
    aff4:chunks_per_segment 1024 ;
    aff4:compression <https://www.ietf.org/rfc/rfc1951.txt> ;
    aff4:size 1779 ;
    aff4:stored <aff4://21803da1-a4e4-4cf7-b866-f4f94c6fb041> ;
    a aff4:image .
```

This clause contains a number of statements about an AFF4 object:

1. The AFF4 object we are talking about has a URN of
*aff4://21803da1-a4e4-4cf7-b866-f4f94c6fb041/bin/zcmp*. Note that this URN is
globally unique but we can see that it represents a file that was found at a
path of */bin/zcmp*.  The choice for the URN is completely up to the
implementation. The only requirement is that the URN is globally unique - this
implementation chose to append the source filename to the URN for a bit of
context but this is only a convenience.

2. We can see that the object is an *aff4:image* (more on this later). This
simply signifies what type of object this is.

3. The object is stored in a volume of URN
*aff4://21803da1-a4e4-4cf7-b866-f4f94c6fb041*. More on this later.

### Object oriented modeling

We said before that AFF4 is an object oriented model. What does that mean? The
AFF4 information model describes information about AFF4 objects. AFF4 objects
are real implementations which can be instantiated by AFF4 implementations from
the information in the model.

We currently define a number of standard AFF4 objects. Each standard object
contains a set of pre-defined predicates which can be used to describe it. An
AFF4 implementation can read the information model and re-create an AFF4 object
from the RDF triples. Standard AFF4 objects specify their behaviour in
sufficient details so that different AFF4 implementations can interchange data
freely.

### Implicit relations

Since AFF4 is reliant on the information model, all AFF4 objects are described
using RDF triples. However, in practice many triples are implicitly
stated. Either because they take on a default value, or because their facts are
obvious.

For example, the AFF4 predicate *aff4:stored* describes where an AFF4 object is
stored. However, when parsing an AFF4 volume we often already know some objects
contained within it, simply by virtue of the volume format itself. It is
therefore not needed to state these triples explicitely.

AFF4 standard objects often define implicit relations and therefore allow some
RDF statement to be deduced by context. In the following we will describe what
triples may be omitted for each AFF4 object type.