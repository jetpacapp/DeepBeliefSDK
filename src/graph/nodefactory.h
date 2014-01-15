//
//  nodefactory.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_NODEFACTORY_H
#define INCLUDE_NODEFACTORY_H

#include "jpcnn.h"
#include "binary_format.h"

class BaseNode;

BaseNode* new_node_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_NODEFACTORY_H