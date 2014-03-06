// Copyright 2014 Jetpac Inc
// All rights reserved.
// Pete Warden <pete@jetpac.com>

Dimensions = function(input) {
  if (input instanceof Array) {
    this._dims = _.clone(input);
  } else if (input instanceof Dimensions) {
    this._dims = _.clone(input._dims);
  } else {
    var argsAsArray = Array.prototype.slice.call(arguments, 0);
    this._dims = _.clone(argsAsArray);
  }

  // See http://stackoverflow.com/questions/18082/validate-numbers-in-javascript-isnumeric
  function isNumber(n) {
    return !isNaN(parseFloat(n)) && isFinite(n);
  }
  _.each(this._dims, function(dim) {
    if (!isNumber(dim)) {
      throw "Unknown input type to Dimensions() - " + input;
    }
  });
};
Dimensions.prototype.elementCount = function() {
  var result = 1;
  for (var index = 0; index < this._dims.length; index += 1) {
    result *= this._dims[index];
  }
  return result;
};
Dimensions.prototype.offset = function(indices) {
  if (!(indices instanceof Array)) {
    var argsAsArray = Array.prototype.slice.call(arguments, 0);
    indices = argsAsArray;
  }
  var reverseIndices = _.clone(indices).reverse();
  var reverseDims = _.clone(this._dims).reverse();
  var size = 1;
  var total = 0;
  _.each(reverseDims, function(dim, loopIndex) {
    var index = reverseIndices[loopIndex];
    total += (index * size);
    size *= dim;
  });
  return total;
};
Dimensions.prototype.toString = function() {
  return '(' + this._dims.join(', ') + ')';
};
Dimensions.prototype.removeDimensions = function(howMany) {
  return new Dimensions(this._dims.slice(howMany));
};
Dimensions.prototype.areEqualTo = function(other) {
  if (this._dims.length != other._dims.length) {
    return false;
  }
  for (var index = 0; index < this._dims.length; index += 1) {
    if (this._dims[index] != other._dims[index]) {
      return false;
    }
  }
  return true;
}

Buffer = function(dims, data) {
  this._dims = new Dimensions(dims);
  if (_.isUndefined(data)) {
    var elementCount = this._dims.elementCount();
    this._data = new Float32Array(elementCount);
  } else {
    this._data = data;
  }
  this._name = 'None';
};
Buffer.prototype.canReshapeTo = function(newDims) {
  // TO DO
};
Buffer.prototype.reshape = function(newDims) {
  console.assert((newDims.elementCount() === this._dims.elementCount()), 'reshape() must have the same number of elements');
  this._dims = newDims;
  return this;
};
Buffer.prototype.copyDataFrom = function(other) {
  // TO DO
};
Buffer.prototype.convertFromChannelMajor = function(expectedDims) {
  // TO DO
};
Buffer.prototype.populateWithRandomValues = function(min, max) {
  // TO DO
};
Buffer.prototype.view = function() {
  var result = new Buffer(this._dims, this._data);
  result.setName(this._name + ' view');
  return result;
};
Buffer.prototype.toString = function() {
  return 'Buffer "' + this._name + '" ' + this._dims;
};
Buffer.prototype.printContents = function(maxElements) {
  // TO DO
};
Buffer.prototype.showDebugImage = function() {
  var dims = this._dims._dims;
  if (dims.length == 3) {
    var width = dims[1];
    var height = dims[0];
    var channels = dims[2];
    var canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;
    var context = canvas.getContext("2d");
    for (var y = 0; y < height; y += 1) {
      for (var x = 0; x < width; x += 1) {
        var pixelColors = [0, 0, 0, 1];
        for (var channel = 0; channel < channels; channel += 1) {
          var value = Math.floor(this.valueAt(imageCount, y, x, channel));
          pixelColors[channel] = value;
        }
        var color = 'rgba(' + pixelColors.join(',') + ')';
        context.fillStyle = color;
        context.fillStyle = 'rgba(' + pixelColors.join(',') + ')';
        context.fillRect(x, y, 1, 1);
      }
    }
    var dataURL = canvas.toDataURL('image/png');
    console.log(dataURL);
    document.write('<img src="' + dataURL + '"/><br/>');
    window.open(dataURL, '_blank');
  } else if (dims.length == 4) {
    var imageCount = dims[0];
    for (var imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
      var width = dims[2];
      var height = dims[1];
      var channels = dims[3];
      var canvas = document.createElement("canvas");
      canvas.width = width;
      canvas.height = height;
      var context = canvas.getContext("2d");
      for (var y = 0; y < height; y += 1) {
        for (var x = 0; x < width; x += 1) {
          var pixelColors = [0, 0, 0, 1];
          for (var channel = 0; channel < channels; channel += 1) {
            var value = Math.floor(this.valueAt(imageIndex, y, x, channel));
            pixelColors[channel] = value;
          }
          var color = 'rgba(' + pixelColors.join(',') + ')';
          context.fillStyle = color;
          context.fillRect(x, y, 1, 1);
        }
      }
      var dataURL = canvas.toDataURL("image/png");
      console.log(dataURL);
      document.write('<img src="' + dataURL + '"/><br/>');
      window.open(dataURL, '_blank');
    }
  } else {
    console.log('Unknown image size');
  }
};
Buffer.prototype.setName = function(name) {
  this._name = name;
};
Buffer.prototype.fromTagDict = function(mainDict, skipCopy) {
  // TO DO
};
Buffer.prototype.areAllClose = function(other, tolerance) {
  // TO DO
};
Buffer.prototype.convertFromChanneledRGBImage = function() {
  // TO DO
};
Buffer.prototype.convertToChanneledRGBImage = function() {
  // TO DO
};
Buffer.prototype.extractSubRegion = function(origin, size) {
  // TO DO
};
Buffer.prototype.valueAt = function() {
  var argsAsArray = Array.prototype.slice.call(arguments, 0);
  var elementOffset = this._dims.offset(argsAsArray);
  return this._data[elementOffset];
};
Buffer.prototype.viewAtTopIndex = function(index) {
  var inputDims = this._dims;
  console.assert(inputDims._dims.length > 1);
  console.assert(index < inputDims._dims[0]);
  var outputDims = inputDims.removeDimensions(1);
  var topStride = outputDims.elementCount();
  var viewData = this._data.subarray((topStride * index), (topStride * (index + 1)));
  var output = new Buffer(outputDims, viewData);
  return output;
};
function bufferFromTagDict(tagDict) {
  console.assert(tagDict.type === JP_DICT);
  var bitsPerFloat = tagDict.getUintFromDict('float_bits');
  console.assert(bitsPerFloat === 32);
  var dimsTag = tagDict.getTagFromDict('dims');
  var dimsSubTags = dimsTag.getSubTags();
  var dimsValues = [];
  _.each(dimsSubTags, function(subTag) {
    console.assert(subTag.type === JP_UINT);
    dimsValues.push(subTag.value);
  });
  var dims = new Dimensions(dimsValues);
  var dataTag = tagDict.getTagFromDict("data");
  console.assert(dataTag.type === JP_FARY);

  var elementCount = dims.elementCount();
  console.assert(dataTag.length === (elementCount * 4));

  var dataTagArray = dataTag.value;
  var buffer = new Buffer(dims, dataTagArray);

  return buffer;
}

Network = function(filename, onLoad) {
  this._isLoaded = false;
  this._isHomebrewed = true;
  this._fileTag = null;
  this._onLoad = onLoad;
  var xhr = new XMLHttpRequest();
  xhr.open('GET', filename, true);
  xhr.responseType = 'arraybuffer';
  xhr.onload = function (myThis) {
    var myNetwork = myThis;
    return function(e) {
      if (this.status == 200) {
      var blob = new Blob([this.response]);
      myNetwork.initializeFromBlob(blob);
      }
    };
  }(this);
  xhr.onerror = function(e) {
    alert("Error " + e.target.status + " occurred while receiving the document.");
  };
  xhr.send();
};
Network.prototype.classifyImage = function(input, doMultiSample, layerOffset) {

  var doFlip;
  var imageSize;
  var isMeanChanneled;
  if (this._isHomebrewed) {
    imageSize = 224;
    doFlip = false;
    isMeanChanneled = true;
  } else {
    imageSize = 227;
    doFlip = true;
    isMeanChanneled = false;
  }
  var rescaledSize = 256;

  var prepareInput = new PrepareInputNode(this._dataMean, !doMultiSample, doFlip, imageSize, rescaledSize, isMeanChanneled);
  var rescaledInput = prepareInput.run(input);
  var predictions = this.run(rescaledInput, layerOffset);

  var result = [];
  for (var index = 0; index < predictions._data.length; index += 1) {
    result.push({value: predictions._data[index], label: this._labelNames[index]});
  }

  return result;
};
Network.prototype.initializeFromBlob = function(blob) {
  var fileReader = new FileReader();
  fileReader.onload = function(myThis) {
    var myNetwork = myThis;
    return function() {
      myNetwork.initializeFromArrayBuffer(this.result)
    };
  }(this);
  fileReader.readAsArrayBuffer(blob);
};
Network.prototype.initializeFromArrayBuffer = function(arrayBuffer) {
  this.binaryFormat = new BinaryFormat(arrayBuffer);
  var graphDict = this.binaryFormat.firstTag();
  this._fileTag = graphDict;
  var dataMeanTag = graphDict.getTagFromDict('data_mean');
  console.assert(dataMeanTag != null);
  this._dataMean = bufferFromTagDict(dataMeanTag);

  var layersTag = graphDict.getTagFromDict('layers');
  var layerSubTags = layersTag.getSubTags();
  var layers = [];
  _.each(layerSubTags, function(layerSubTag) {
    var layerNode = nodeFromTag(layerSubTag);
    layers.push(layerNode);
  });
  this._layers = layers;

  var labelNamesTag = graphDict.getTagFromDict('label_names');
  var labelNameSubTags = labelNamesTag.getSubTags();
  var labelNames = [];
  _.each(labelNameSubTags, function(labelNameTag) {
    labelNames.push(labelNameTag.value);
  });
  this._labelNames = labelNames;

  this._onLoad(this);
};
Network.prototype.run = function(input, layerOffset) {
  if (_.isUndefined(layerOffset)) {
    layerOffset = 0;
  }
  var currentInput = input;
  var howManyLayers = (this._layers.length + layerOffset);
  for (var index = 0; index < howManyLayers; index += 1) {
    var layer = this._layers[index];
    console.log('Running ' + layer.constructor.name)
    var currentOutput = layer.run(currentInput);
    currentOutput.setName(layer.constructor.name + ' output');
    console.log('currentOutput = ' + currentOutput);
    currentInput = currentOutput;
  }
  return currentInput;
};

BinaryFormat = function(arrayBuffer) {
  this.arrayBuffer = arrayBuffer;
  this.cursor = 0;
};
JP_CHAR = 0x52414843; // 'CHAR'
JP_UINT = 0x544E4955; // 'UINT'
JP_FL32 = 0x32334C46; // 'FL32'
JP_FARY = 0x59524146; // 'FARY'
JP_DICT = 0x54434944; // 'DICT'
JP_LIST = 0x5453494C; // 'LIST'
BinaryFormat.prototype.firstTag = function() {
  return tagFromMemory(this.arrayBuffer, 0);
};

function tagFromMemory(arrayBuffer, offset) {
  var header = new Uint32Array(arrayBuffer, offset, 2);
  var type = header[0];
  var length = header[1];
  var valuesBuffer = arrayBuffer.slice((offset + 8), (offset + 8 + length));
  return new BinaryTag(type, length, valuesBuffer);
};

BinaryTag = function(type, length, valuesBuffer) {
  var value;
  if (type === JP_CHAR) {
    var stringBytes = new Uint8Array(valuesBuffer, 0, (length-1));
    value = '';
    var index = 0;
    while (index < (length - 1)) {
      var charCode = stringBytes[index];
      if (charCode === 0) {
        break;
      }
      value += String.fromCharCode(charCode);
      index += 1;
    }
  } else if (type === JP_UINT) {
    var array = new Uint32Array(valuesBuffer, 0, 1);
    value = array[0];
  } else if (type === JP_FL32) {
    var array = new Float32Array(valuesBuffer, 0, 1);
    value = array[0];
  } else if (type === JP_FARY) {
    var array = new Float32Array(valuesBuffer, 0, (length / 4));
    value = array;
  } else if (type === JP_DICT) {
    value = valuesBuffer;
  } else if (type === JP_LIST) {
    value = valuesBuffer;
  } else {
    console.log('Unknown type ' + type);
    return null;
  }
  this.type = type;
  this.length = length;
  this.value = value;
};
BinaryTag.prototype.toString = function() {
  var type = this.type;
  var length = this.length;
  var name;
  if (type === JP_CHAR) {
    name = 'CHAR';
  } else if (type === JP_UINT) {
    name = 'UINT';
  } else if (type === JP_FL32) {
    name = 'FL32';
  } else if (type === JP_FARY) {
    name = 'FARY';
  } else if (type === JP_DICT) {
    name = 'DICT';
  } else if (type === JP_LIST) {
    name = 'LIST';
  } else {
    console.log('Unknown type ' + type);
    return null;
  }
  return 'Tag ' + name + ', length=' + this.length + ', value = ' + this.value;
};
BinaryTag.prototype.sizeInBytes = function() {
  return (8 + this.length);
};
BinaryTag.prototype.getSubTags = function() {
  console.assert((this.type === JP_DICT) || (this.type === JP_LIST));
  var valuesBuffer = this.value;
  var length = this.length;
  var readOffset = 0;
  var result = [];
  while (readOffset < length) {
    var tag = tagFromMemory(valuesBuffer, readOffset);
    result.push(tag);
    readOffset += tag.sizeInBytes();
  }
  return result;
}
BinaryTag.prototype.getTagFromDict = function(wantedKey) {
  var result = null;
  var subTags = this.getSubTags();
  console.assert((subTags.length % 2) == 0);
  for (var index = 0; index < subTags.length; index += 2) {
    var key = subTags[index + 0];
    console.assert((key.type === JP_CHAR), 'Key must be a string');
    if (key.value == wantedKey) {
      result = subTags[index + 1];
    }
  }
  return result;
};
BinaryTag.prototype.getStringFromDict = function(wantedKey) {
  var tag = this.getTagFromDict(wantedKey);
  console.assert(tag && tag.type == JP_CHAR);
  return tag.value;
};
BinaryTag.prototype.getUintFromDict = function(wantedKey) {
  var tag = this.getTagFromDict(wantedKey);
  console.assert(tag && tag.type == JP_UINT);
  return tag.value;
};
BinaryTag.prototype.getFloatFromDict = function(wantedKey) {
  var tag = this.getTagFromDict(wantedKey);
  console.assert(tag && tag.type == JP_FL32);
  return tag.value;
};

function nodeFromTag(tag) {
  var classLookup = {
    'conv': ConvNode,
    'dropout': DropoutNode,
    'flat': FlatNode,
    'gconv': GConvNode,
    'neuron': NeuronNode,
    'normalize': NormalizeNode,
    'pool': PoolNode,
    'relu': ReluNode,
    'max': MaxNode,
  };
  var tagClass = tag.getStringFromDict('class');
  var jsClass = classLookup[tagClass];
  var result = new jsClass(tag);
  return result;
}

function ConvNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'conv', 'Wrong class name in tag');

  var specDict = tag.getTagFromDict('spec');
  this._kernelCount = specDict.getUintFromDict('num_kernels');
  this._kernelWidth = specDict.getUintFromDict('ksize');
  this._sampleStride = specDict.getUintFromDict('stride');

  var kernelsTag = tag.getTagFromDict('kernels');
  this._kernels = bufferFromTagDict(kernelsTag);

  this._useBias = tag.getUintFromDict('has_bias');
  if (this._useBias) {
    var biasTag = tag.getTagFromDict('bias');
    this._bias = bufferFromTagDict(biasTag);
  }

  this._marginSize = tag.getUintFromDict('padding');
}
ConvNode.prototype.run = function(input) {
  var inputDims = input._dims;
  var inputChannels = inputDims._dims[inputDims._dims.length - 1];
  var valuesPerKernel = (inputChannels * this._kernelWidth * this._kernelWidth);
  var expectedKernelsDims = new Dimensions(valuesPerKernel, this._kernelCount);
  console.assert(expectedKernelsDims.areEqualTo(this._kernels._dims));

  var inputWithMargin;
  if (this._marginSize == 0) {
    inputWithMargin = input;
  } else {
    inputWithMargin = matrixInsertMargin(input, this._marginSize, this._marginSize);
  }

  this._output = matrixCorrelate(inputWithMargin, this._kernels, this._kernelWidth, this._kernelCount, this._sampleStride);
  this._output.setName(this._name);

  matrixAddInplace(this._output, this._bias, 1.0);

  return this._output;
};

function DropoutNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'dropout', 'Wrong class name in tag');
}
DropoutNode.prototype.run = function(input) {
  return input;
};

function FlatNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'flat', 'Wrong class name in tag');
}
FlatNode.prototype.run = function(input) {
  var inputDims = input._dims;
  // We're expecting (# of images, height, width, # of channels)
  console.assert(inputDims._dims.length == 4);

  var imageCount = inputDims._dims[0];
  var inputWidth = inputDims._dims[2];
  var inputHeight = inputDims._dims[1];
  var inputChannels = inputDims._dims[3];

  var outputElementCount = (inputHeight * inputWidth * inputChannels);
  var outputDims = new Dimensions(imageCount, outputElementCount);

  // Doesn't do a data copy, just returns a new view with a different shape.
  this._output = new Buffer(outputDims, input._data);

  return this._output;
};

function GConvNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'gconv', 'Wrong class name in tag');

  this._subnodesCount = tag.getUintFromDict('layers_count');
  var subnodesTag = tag.getTagFromDict('layers');
  var subnodeSubTags = subnodesTag.getSubTags();
  var subnodes = [];
  _.each(subnodeSubTags, function(subnodeSubTag) {
    var subnode = nodeFromTag(subnodeSubTag);
    subnodes.push(subnode);
  });
  this._subnodes = subnodes;

  this._kernelCount = tag.getUintFromDict('kernels_count');
}
GConvNode.prototype.run = function(input) {
  var inputDims = input._dims;

  console.assert(inputDims._dims.length === 4);

  var imageCount = inputDims._dims[0];
  var inputWidth = inputDims._dims[2];
  var inputHeight = inputDims._dims[1];
  var inputChannels = inputDims._dims[3];

  console.assert((inputChannels % this._subnodesCount) === 0);
  var subnodeChannels = (inputChannels / this._subnodes.length);

  var subnodeInputDimensions = new Dimensions(imageCount, inputHeight, inputWidth, subnodeChannels);
  var subnodeOutputBuffers = [];

  for (var index = 0; index < this._subnodes.length; index += 1) {
    var startChannel = (index * subnodeChannels);
    var endChannel = ((index + 1) * subnodeChannels);
    var subnodeInputBuffer = matrixExtractChannels(input, startChannel, endChannel);

    var subnode = this._subnodes[index];
    var subnodeOutputBuffer = subnode.run(subnodeInputBuffer);
    subnodeOutputBuffers.push(subnodeOutputBuffer);
  }

  this._output = matrixJoinChannels(subnodeOutputBuffers);

  return this._output;
};

function NeuronNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'neuron', 'Wrong class name in tag');

  var specDict = tag.getTagFromDict('spec');
  this._outputsCount = specDict.getUintFromDict('num_output');

  var weightsTag = tag.getTagFromDict('weight');
  this._weights = bufferFromTagDict(weightsTag);

  this._useBias = tag.getUintFromDict('has_bias');
  if (this._useBias) {
    var biasTag = tag.getTagFromDict('bias');
    this._bias = bufferFromTagDict(biasTag);
  }

  if (tag.getTagFromDict('dropout')) {
    this._dropout = tag.getFloatFromDict('dropout');
  }
}
NeuronNode.prototype.run = function(input) {
  var inputDims = input._dims;
  var numberOfImages = inputDims._dims[0];
  var inputImageDims = inputDims.removeDimensions(1);
  var elementCount = inputImageDims.elementCount();
  var flattenedDimensions = new Dimensions(numberOfImages, elementCount);
  var flattenedInput = input.view();
  flattenedInput.reshape(flattenedDimensions);

  var expectedWeightsDimensions = new Dimensions(elementCount, this._outputsCount);
  console.assert(expectedWeightsDimensions.areEqualTo(this._weights._dims));

  this._output = matrixDot(flattenedInput, this._weights);
  this._output.setName(this._name);

  matrixAddInplace(this._output, this._bias, 1.0);

  if (this._dropout > 0.0) {
    var scale = (1.0 - this._dropout);
    matrixScaleInplace(this._output, scale);
  }

  return this._output;
};

function NormalizeNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'normalize', 'Wrong class name in tag');

  this._windowSize = tag.getUintFromDict('size');
  this._k = tag.getFloatFromDict('k');
  this._alpha = tag.getFloatFromDict('alpha');
  this._beta = tag.getFloatFromDict('beta');
}
NormalizeNode.prototype.run = function(input) {
  this._output = matrixLocalResponse(input, this._windowSize, this._k, this._alpha, this._beta);
  return this._output;
};

function PoolNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'pool', 'Wrong class name in tag');

  this._patchWidth = tag.getUintFromDict('psize');
  this._stride = tag.getUintFromDict('stride');
  this._mode = tag.getStringFromDict('mode');
}
PoolNode.prototype.run = function(input) {
  this._output = matrixMaxPatch(input, this._patchWidth, this._stride);
  return this._output;
};

function ReluNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'relu', 'Wrong class name in tag');
}
ReluNode.prototype.run = function(input) {
  this._output = matrixMax(input, 0.0);
  return this._output;
};

function MaxNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'max', 'Wrong class name in tag');
}
MaxNode.prototype.run = function(input) {
  this._output = matrixSoftmax(input);
  return this._output;
};

function PrepareInputNode(dataMean, useCenterOnly, needsFlip, imageSize, rescaledSize, isMeanChanneled) {
  this._useCenterOnly = useCenterOnly;
  this._needsFlip = needsFlip;
  this._imageSize = imageSize;
  this._rescaledSize = rescaledSize;
  var expectedDims = new Dimensions(this._rescaledSize, this._rescaledSize, 3);
  dataMean.reshape(expectedDims);
  var outputDims = new Dimensions(this._imageSize, this._imageSize, 3);
  this._dataMean = new Buffer(outputDims);
  var deltaX = (this._rescaledSize - this._imageSize);
  var deltaY = (this._rescaledSize - this._imageSize);
  var marginX = (deltaX / 2);
  var marginY = (deltaY / 2);
  if (isMeanChanneled) {
    var fromChanneled = convertFromChanneledRGBImage(dataMean);
    cropAndFlipImage(this._dataMean, fromChanneled, marginX, marginY, false);
  } else {
    cropAndFlipImage(this._dataMean, dataMean, marginX, marginY, false);
  }
  this._dataMean.setName('_dataMean');
}
PrepareInputNode.prototype.run = function(input) {
  var rescaledDims = new Dimensions(this._rescaledSize, this._rescaledSize, 3);
  console.assert(input._dims.areEqualTo(rescaledDims));
  console.assert(!this._needsFlip);

  input.setName("input");

  var deltaX = (this._rescaledSize - this._imageSize);
  var deltaY = (this._rescaledSize - this._imageSize);
  var marginX = (deltaX / 2);
  var marginY = (deltaY / 2);

  if (this._useCenterOnly) {

    var outputDims = new Dimensions(1, this._imageSize, this._imageSize, 3);
    this._output = new Buffer(outputDims);
    this._output.setName("prepareInput_output");

    var sourceX = marginX;
    var sourceY = marginY;

    var blitDestination = this._output.viewAtTopIndex(0);
    cropAndFlipImage(blitDestination, input, sourceX, sourceY, false);

    matrixAddInplace(blitDestination, this._dataMean, -1.0);

  } else {

    var outputDims = new Dimensions(10, this._imageSize, this._imageSize, 3);
    this._output = new Buffer(outputDims);
    this._output.setName('prepareInput_output');

    for (var flipPass = 0; flipPass < 2; flipPass += 1) {
      var doFlip = (flipPass == 1);
      var blitDestination = this._output.viewAtTopIndex(this._output, (flipPass * 5));
      cropAndFlipImage(blitDestination, rescaled, marginX, marginY, doFlip);
      for (var yIndex = 0; yIndex < 2; yIndex += 1) {
        for (var xIndex = 0; xIndex < 2; xIndex += 1) {
          var viewIndex = ((flipPass * 5) + (yIndex * 2) + xIndex + 1);
          var blitDestination = bufferViewAtTopIndex(this._output, viewIndex);

          var sourceX = (xIndex * deltaX);
          var sourceY = (yIndex * deltaY);

          cropAndFlipImage(blitDestination, input, sourceX, sourceY, doFlip);
        }
      }
    }
  }

  return this._output;
};

function cropAndFlipImage(destBuffer, sourceBuffer, offsetX, offsetY, doFlipHorizontal) {

  var destDims = destBuffer._dims;
  var sourceDims = sourceBuffer._dims;
  console.assert((destDims._dims.length == 3) && (sourceDims._dims.length == 3));

  var destWidth = destDims._dims[1];
  var destHeight = destDims._dims[0];
  var destChannels = destDims._dims[2];

  var sourceWidth = sourceDims._dims[1];
  var sourceHeight = sourceDims._dims[0];
  var sourceChannels = sourceDims._dims[2];
  console.assert(destChannels == sourceChannels);

  var sourceEndX = (offsetX + destWidth);
  console.assert(sourceEndX <= sourceWidth);
  var sourceEndY = (offsetY + destHeight);
  console.assert(sourceEndY <= sourceHeight);

  var destRowDims = destDims.removeDimensions(1);
  var destRowElementCount = destRowDims.elementCount();

  var destData = destBuffer._data;
  var sourceData = sourceBuffer._data;

  if (!doFlipHorizontal) {
    for (var destY = 0; destY < destHeight; destY += 1) {
      var sourceX = offsetX;
      var sourceY = (destY + offsetY);
      var sourceOffset = sourceDims.offset(sourceY, sourceX, 0);
      var sourceRow = sourceData.subarray(sourceOffset, (sourceOffset + destRowElementCount));
      var destOffset = destDims.offset(destY, 0, 0);
      var destRow = destData.subarray(destOffset, (destOffset + destRowElementCount));
      destRow.set(sourceRow);
    }
  } else {
    for (var destY = 0; destY < destHeight; destY += 1) {
      var sourceX = offsetX;
      var sourceY = (destY + offsetY);
      var sourceOffset = sourceDims.offset(sourceY, sourceX, 0);
      var sourceRow = sourceData.subarray(sourceOffset, (sourceOffset + destRowElementCount));
      var destOffset = destDims.offset(destY, 0, 0);
      var destRow = destData.subarray(destOffset, destRowElementCount);
      for (var destX = 0; destX < destWidth; destX += 1) {
        var sourceX = ((destWidth - 1) - destX);
        var sourcePixel = sourceRow.subarray((sourceX * sourceChannels), ((sourceX * sourceChannels) + destChannels));
        var destPixel = sourceRow.subarray((destX * destChannels), ((destX * destChannels) + destChannels));
        destPixel.set(sourcePixel);
      }
    }
  }
}

function convertFromChanneledRGBImage(input) {
  var dims = input._dims;
  console.assert(dims._dims.length == 3);
  var width = dims._dims[1];
  var height = dims._dims[0];
  var channels = dims._dims[2];
  console.assert(channels == 3);

  var result = new Buffer(dims);

  var inputData = input._data;
  var outputData = result._data;

  var elementsPerChannel = (width * height);
  var elementsPerImage = dims.elementCount();

  var redOffset = (0 * elementsPerChannel);
  var greenOffset = (1 * elementsPerChannel);
  var blueOffset = (2 * elementsPerChannel);

  var destOffset = 0;
  while (destOffset < elementsPerImage) {
    outputData[destOffset] = inputData[redOffset];
    redOffset += 1;
    destOffset += 1;
    outputData[destOffset] = inputData[greenOffset];
    greenOffset += 1;
    destOffset += 1;
    outputData[destOffset] = inputData[blueOffset];
    blueOffset += 1;
    destOffset += 1;
  }

  return result;
}

function matrixAddInplace(output, input, inputScale) {
  console.assert((output._dims.elementCount() % input._dims.elementCount()) == 0);
  var outputData = output._data;
  var outputDataLength = output._dims.elementCount();
  var inputData = input._data;
  var inputDataLength = input._dims.elementCount();

  var outputOffset = 0;
  var inputOffset = 0;
  while (outputOffset < outputDataLength) {
    var inputValue = inputData[inputOffset];
    outputData[outputOffset] += (inputValue * inputScale);
    outputOffset += 1;
    inputOffset += 1;
    if (inputOffset >= inputDataLength) {
      inputOffset = 0;
    }
  }
}

function matrixInsertMargin(input, marginWidth, marginHeight) {

  var inputDims = input._dims;
  // We're expecting (# of images, height, width, # of channels)
  console.assert(inputDims._dims.length == 4);

  var imageCount = inputDims._dims[0];
  var inputWidth = inputDims._dims[2];
  var inputHeight = inputDims._dims[1];
  var inputChannels = inputDims._dims[3];

  var outputWidth = (inputWidth + (marginWidth * 2));
  var outputHeight = (inputHeight + (marginHeight * 2));
  var outputDims = new Dimensions(imageCount, outputHeight, outputWidth, inputChannels);
  var output = new Buffer(outputDims);

  var valuesPerInputRow = (inputWidth * inputChannels);
  var valuesPerOutputRow = (outputWidth * inputChannels);

  var valuesPerOutputMargin = (marginWidth * inputChannels);

  var outputData = output._data;
  var outputOffset = 0;
  var inputData = input._data;
  var inputOffset = 0;

  for (var imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (var outputY = 0; outputY < outputHeight; outputY += 1) {
      var inputOriginY = (outputY - marginHeight);
      if ((inputOriginY < 0) || (inputOriginY >= inputHeight)) {
        outputOffset += valuesPerOutputRow;
      } else {
        outputOffset += valuesPerOutputMargin;
        var inputRow = inputData.subarray(inputOffset, (inputOffset + valuesPerInputRow));
        outputData.set(inputRow, outputOffset);
        outputOffset += valuesPerInputRow;
        inputOffset += valuesPerInputRow;
        outputOffset += valuesPerOutputMargin;
      }
    }
  }

  return output;
}

function patchesIntoRows(input, kernelWidth, stride) {

  var inputDims = input._dims;
  // We're expecting (# of images, height, width, # of channels)
  console.assert(inputDims._dims.length == 4);

  var imageCount = inputDims._dims[0];
  var inputWidth = inputDims._dims[2];
  var inputHeight = inputDims._dims[1];
  var inputChannels = inputDims._dims[3];

  var pixelsPerKernel = (kernelWidth * kernelWidth);
  var valuesPerKernel = (pixelsPerKernel * inputChannels);

  var patchesAcross = Math.round(Math.ceil((inputWidth - kernelWidth) / stride) + 1);
  var patchesDown = Math.round(Math.ceil((inputHeight - kernelWidth) / stride) + 1);
  var outputDims = new Dimensions(imageCount, (patchesDown * patchesAcross), valuesPerKernel);
  var output = new Buffer(outputDims);

  var inputData = input._data;
  var outputData = output._data;
  var outputOffset = 0;

  var valuesPerInputRow = inputDims.removeDimensions(2).elementCount();
  var valuesPerKernelRow = (kernelWidth * inputChannels);

  for (var imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (var patchY = 0; patchY < patchesDown; patchY += 1) {
      var inputOriginY = (patchY * stride);
      var inputEndY = (inputOriginY + kernelWidth);
      for (var patchX = 0; patchX < patchesAcross; patchX += 1) {
        var inputOriginX = (patchX * stride);
        var inputEndX = (inputOriginX + kernelWidth);
        var inputOffset = inputDims.offset(imageIndex, inputOriginY, inputOriginX, 0);
        if ((inputEndY <= inputHeight) && (inputEndX <= inputWidth)) {
          for (var row = 0; row < kernelWidth; row += 1) {
            var inputRow = inputData.subarray(inputOffset, (inputOffset + valuesPerKernelRow));
            outputData.set(inputRow, outputOffset);
            outputOffset += valuesPerKernelRow;
            inputOffset += valuesPerInputRow;
          }
        } else {
          var valuesToCopy;
          if (inputEndX > inputWidth) {
            valuesToCopy = ((inputEndX - inputWidth) * inputChannels);
          } else {
            valuesToCopy = valuesPerKernelRow;
          }
          var valuesToZero = (valuesPerKernelRow - valuesToCopy);
          var rowsToCopy;
          if (inputEndY > inputHeight) {
            rowsToCopy = (kernelWidth - (inputEndY - inputHeight));
          } else {
            rowsToCopy = kernelWidth;
          }
          for (var row = 0; row < kernelWidth; row += 1) {
            if (row < rowsToCopy) {
              var inputRow = inputData.subarray(inputOffset, (inputOffset + valuesToCopy));
              outputData.set(inputRow, outputOffset);
              outputOffset += valuesPerKernelRow;
              inputOffset += valuesPerInputRow;
            } else {
              outputOffset += valuesPerKernelRow;
            }
          }
        }
      }
    }
  }

  return output;
}

function matrixCorrelate(input, kernels, kernelWidth, kernelCount, stride) {

  var inputDims = input._dims;
  // We're expecting (# of images, height, width, # of channels)
  console.assert(inputDims._dims.length == 4);

  var imageCount = inputDims._dims[0];
  var inputWidth = inputDims._dims[2];
  var inputHeight = inputDims._dims[1];
  var inputChannels = inputDims._dims[3];

  var pixelsPerKernel = (kernelWidth * kernelWidth);
  var valuesPerKernel = (pixelsPerKernel * inputChannels);
  var expectedKernelsDims = new Dimensions(valuesPerKernel, kernelCount);
  console.assert(expectedKernelsDims.areEqualTo(kernels._dims));

  var outputWidth = Math.round(Math.ceil((inputWidth - kernelWidth) / stride) + 1);
  var outputHeight = Math.round(Math.ceil((inputHeight - kernelWidth) / stride) + 1);
  var outputChannels = kernelCount;
  var outputDims = new Dimensions(imageCount, outputHeight, outputWidth, outputChannels);
  var output = new Buffer(outputDims);

  var patches = patchesIntoRows(input, kernelWidth, stride);

  var m = kernelCount;
  var n = (patches._dims._dims[1] * patches._dims._dims[0]);
  var k = patches._dims._dims[2];
  var alpha = 1.0;
  var lda = m;
  var ldb = k;
  var ldc = m;
  var beta = 0.0;

  matrixGemm(
    m,
    n,
    k,
    alpha,
    kernels._data,
    lda,
    patches._data,
    ldb,
    beta,
    output._data,
    ldc
  );

  return output;
}

function matrixGemm(
  m,
  n,
  k,
  alpha,
  a,
  lda,
  b,
  ldb,
  beta,
  c,
  ldc) {

  naiveGemm(
    m,
    n,
    k,
    alpha,
    a,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc
  );

}

function naiveGemm(
  m,
  n,
  k,
  alpha,
  a,
  lda,
  b,
  ldb,
  beta,
  c,
  ldc) {

  for (var i = 0; i < m; i++) {
    for (var j = 0; j < n; j++) {
      var total = 0.0;
      for (var l = 0; l < k; l++) {
        var aIndex = ((lda * l) + i);
        var aValue = a[aIndex];
        var bIndex = ((ldb * j) + l);
        var bValue = b[bIndex];
        total += (aValue * bValue);
      }
      var cIndex = ((ldc * j) + i);
      var oldCValue = c[cIndex];
      c[cIndex] = ((alpha * total) + (beta * oldCValue));
    }
  }
}

function matrixExtractChannels(input, startChannel, endChannel) {

  var inputDims = input._dims;
  var inputChannels = inputDims._dims[inputDims._dims.length - 1];
  var outputChannels = (endChannel - startChannel);

  console.assert((inputChannels % outputChannels) == 0);

  var outputDims = new Dimensions(inputDims);
  outputDims._dims[outputDims._dims.length - 1] = outputChannels;
  var output = new Buffer(outputDims);

  var inputData = input._data;
  var inputOffset = startChannel;
  var inputElementCount = inputDims.elementCount();
  var outputData = output._data;
  var outputOffset = 0;

  while (inputOffset < inputElementCount) {
    var sourceRow = inputData.subarray(inputOffset, (inputOffset + outputChannels));
    outputData.set(sourceRow, outputOffset);
    inputOffset += inputChannels;
    outputOffset += outputChannels;
  }

  return output;
}

function matrixJoinChannels(inputs) {
  var inputsCount = inputs.length;
  var firstInput = inputs[0];
  var inputDims = firstInput._dims;
  var inputChannels = inputDims._dims[inputDims._dims.length - 1];
  var outputChannels = (inputChannels * inputsCount);

  var outputDims = new Dimensions(inputDims);
  outputDims._dims[outputDims._dims.length - 1] = outputChannels;
  var output = new Buffer(outputDims);

  var inputDatas = [];
  for (var index = 0; index < inputsCount; index += 1) {
    console.assert(inputs[index]._dims.areEqualTo(inputDims));
    inputDatas.push({data: inputs[index]._data, offset: 0});
  }

  var outputData = output._data;
  var outputOffset = 0;
  var outputElementCount = outputDims.elementCount();

  while (outputOffset < outputElementCount) {
    for (var index = 0; index < inputsCount; index += 1) {
      var input = inputDatas[index];
      var sourceRow = input.data.subarray(input.offset, (input.offset + inputChannels));
      outputData.set(sourceRow, outputOffset);
      input.offset += inputChannels;
      outputOffset += inputChannels;
    }
  }

  return output;
}

function matrixDot(input, weights) {

  var inputDims = input._dims;
  // We're expecting (# of images, # of values)
  console.assert(inputDims._dims.length == 2);

  var imageCount = inputDims._dims[0];
  var inputValuesCount = inputDims._dims[1];

  var weightsDims = weights._dims;
  // We're expecting (# of values in input, # of output channels)
  console.assert(inputDims._dims.length == 2);
  console.assert(weightsDims._dims[0] == inputValuesCount);
  var outputChannels = weightsDims._dims[1];

  var outputDims = new Dimensions(imageCount, outputChannels);
  var output = new Buffer(outputDims);

  var m = outputChannels;
  var n = input._dims[0];
  var k = input._dims[1];
  var alpha = 1.0;
  var lda = m;
  var ldb = k;
  var ldc = m;
  var beta = 0.0;

  matrixGemm(
    m,
    n,
    k,
    alpha,
    weights._data,
    lda,
    input._data,
    ldb,
    beta,
    output._data,
    ldc
  );

  return output;
}

function matrixScaleInplace(output, scale) {
  var outputData = output._data;
  var outputDataLength = output._dims.elementCount();
  var outputOffset = 0;
  while (outputOffset < outputDataLength) {
    outputData[outputOffset] *= scale;
    outputOffset += 1;
  }
}

function matrixLocalResponse(input, windowSize, k, alpha, beta) {

  var inputDims = input._dims;
  // We're expecting (# of images, height, width, # of channels)
  console.assert(inputDims._dims.length == 4);

  var inputChannels = inputDims._dims[3];

  var magnitude = new Buffer(inputDims);

  var magBuffer = new Buffer(new Dimensions(inputChannels));
  var magBufferData = magBuffer._data;

  var inputData = input._data;
  var inputOffset = 0;
  var inputElementCount = inputDims.elementCount();
  var magnitudeData = magnitude._data;
  var magnitudeOffset = 0;

  var alphaOverSize = (alpha / windowSize);
  var prereadCount = ((windowSize / 2) - 0);

  while (inputOffset < inputElementCount) {

    for (var channel = 0; channel < inputChannels; channel += 1) {
      var inputValue = inputData[inputOffset + channel];
      magBufferData[channel] = (inputValue * inputValue * alphaOverSize);
    }

    var averagedScale = 0;
    for (var index = 0; index < prereadCount; index += 1) {
      averagedScale += magBufferData[index];
    }

    for (var channel = 0; channel < inputChannels; channel += 1) {
      var rightIndex = (channel + (windowSize / 2));
      if (rightIndex < inputChannels) {
        averagedScale += magBufferData[rightIndex];
      }
      magnitudeData[magnitudeOffset + channel] = (averagedScale + k);
      var leftIndex = (channel - (windowSize / 2));
      if (leftIndex >= 0) {
        averagedScale -= magBufferData[leftIndex];
      }
    }

    inputOffset += inputChannels;
    magnitudeOffset += inputChannels;
  }

  var output = new Buffer(inputDims);

  inputOffset = 0;
  magnitudeOffset = 0;
  var outputData = output._data;
  while (inputOffset < inputElementCount) {

    var inputValue = inputData[inputOffset];
    var magnitudeValue = magnitudeData[magnitudeOffset];

    var outputValue = (Math.pow(magnitudeValue, -beta) * inputValue);
    if (isNaN(outputValue)) {
      outputValue = 0.0;
    }
    outputData[inputOffset] = outputValue;

    inputOffset += 1;
    magnitudeOffset += 1;
  }

  return output;
}

function matrixMaxPatch(input, patchWidth, stride) {

  var inputDims = input._dims;
  // We're expecting (# of images, height, width, # of channels)
  console.assert(inputDims._dims.length == 4);

  var imageCount = inputDims._dims[0];
  var inputWidth = inputDims._dims[2];
  var inputHeight = inputDims._dims[1];
  var inputChannels = inputDims._dims[3];

  var outputWidth = Math.round(Math.floor((inputWidth - patchWidth) / stride) + 1);
  var outputHeight = Math.round(Math.floor((inputHeight - patchWidth) / stride) + 1);
  var outputChannels = inputChannels;
  var outputDims = new Dimensions(imageCount, outputHeight, outputWidth, outputChannels);
  var output = new Buffer(outputDims);

  var inputData = input._data;
  var outputData = output._data;

  for (var imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (var outputY = 0; outputY < outputHeight; outputY += 1) {
      var inputOriginY = (outputY * stride);
      for (var outputX = 0; outputX < outputWidth; outputX += 1) {
        var inputOriginX = (outputX * stride);
        for (var outputChannel = 0; outputChannel < outputChannels; outputChannel += 1) {
          var patchMax = -Number.MAX_VALUE;
          for (var patchY = 0; patchY < patchWidth; patchY += 1) {
            var inputY = Math.round(Math.min((inputHeight - 1), (inputOriginY + patchY)));
            for (var patchX = 0; patchX < patchWidth; patchX += 1) {
              var inputX = Math.round(Math.min((inputWidth - 1), (inputOriginX + patchX)));
              var inputOffset = inputDims.offset(imageIndex, inputY, inputX, outputChannel);
              var inputValue = input._data[inputOffset];
              patchMax = Math.max(patchMax, inputValue);
            }
          }
          var outputOffset = outputDims.offset(imageIndex, outputY, outputX, outputChannel);
          outputData[outputOffset] = patchMax;
        }
      }
    }
  }

  return output;
}

function matrixMax(input, maxValue) {
  var inputDims = input._dims;
  var output = new Buffer(inputDims);

  var inputData = input._data;
  var inputOffset = 0;
  var inputElementCount = inputDims.elementCount();
  var outputData = output._data;

  while (inputOffset < inputElementCount) {
    var inputValue = inputData[inputOffset];
    outputData[inputOffset] = Math.max(inputValue, maxValue);
    inputOffset += 1;
  }

  return output;
}

function matrixSoftmax(input) {

  var inputDims = input._dims;
  // We're expecting (# of images, # of values)
  console.assert(inputDims._dims.length == 2);

  var imageCount = inputDims._dims[0];
  var inputValuesCount = inputDims._dims[1];

  var output = new Buffer(inputDims);

  var inputData = input._data;
  var outputData = output._data;

  for (var imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    var imageOffsetStart = (imageIndex * inputValuesCount);

    // Rescales the array to accentuate the positive, see here for details:
    // http://stackoverflow.com/questions/9906136/implementation-of-a-softmax-activation-function-for-neural-networks
    var max = -Number.MAX_VALUE;
    var inputOffset = imageOffsetStart;
    while (inputOffset < inputValuesCount) {
      var inputValue = inputData[inputOffset];
      max = Math.max(max, inputValue);
      inputOffset += 1;
    }

    var sum = 0;
    inputOffset = imageOffsetStart;
    while (inputOffset < inputValuesCount) {
      var inputValue = inputData[inputOffset];
      var normalized = (inputValue - max);
      var outputValue = Math.exp(normalized);
      outputData[inputOffset] = outputValue;
      sum += outputValue;
      inputOffset += 1;
    }

    var recipSum = (1.0 / sum);

    inputOffset = imageOffsetStart;
    while (inputOffset < inputValuesCount) {
      outputData[inputOffset] *= recipSum;
      inputOffset += 1;
    }
  }

  return output;
}

