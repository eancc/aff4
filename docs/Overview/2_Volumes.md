---
layout: docs
title: AFF4 Volumes
---

In AFF4 terminology, a *Volume* is a single file which contains other AFF4
objects within itself. The Volume itself is an abstract concept. AFF4 defines a
number of standard implementations for AFF4 volumes, the most common of these if
the *ZipFile* volume which is based on the popular Zip archive format.

Since AFF4 objects are defined both in terms of their information model, and
their data, Volumes typically contain both metadata and actual data. We term a
*Segment* as an atomic piece of data that can be stored in a volume. Segments
are also AFF4 objects and therefore also have globally unique URNs.

## Abstract and Concerete names.

A repeated pattern with the AFF4 design is that it talks about AFF4 objects with
globally unique URNs. However, these objects represent and refer to a real file
with a filename.

Volumes may choose to store and represent the abstract AFF4 object's URN in a
suitable way appropriate to the storage media. Therefore there is always a well
defined mapping between the abstract URN and the concerete name.

For example, if the Volume chooses to store AFF4 objects as files on the
filesystem, the URN may need to be escapes to remove characters which are not
allowed within file names. This escaping scheme (although well defined) is
merely a representational constraint of the AFF4 volume. We still talk about
AFF4 URNs in their abstract form, and then locate them with in the volume.