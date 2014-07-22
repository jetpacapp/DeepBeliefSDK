// Copyright 2014 Jetpac Inc
// All rights reserved.
// Pete Warden <pete@jetpac.com>

/**
 * @constructor
 */
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
window['Dimensions'] = Dimensions;
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

/**
 * @constructor
 */
Buffer = function(dims, data, options) {
  if (_.isUndefined(options)) {
    options = {
      bitsPerFloat: 32,
      min: 0,
      max: 0
    };
  }
  var bitsPerFloat = options.bitsPerFloat;
  var min = options.min;
  var max = options.max;

  this._dims = new Dimensions(dims);
  this._bitsPerFloat = bitsPerFloat;
  this._min = min;
  this._max = max;

  var intRange = (1 << bitsPerFloat);
  var spread = ((max - min) / intRange);
  this._spread = spread;

  if (_.isUndefined(data)) {
    var elementCount = this._dims.elementCount();
    if (bitsPerFloat === 32) {
      this._data = new Float32Array(elementCount);
    } else if (bitsPerFloat === 16) {
      this._quantizedData = new Uint16Array(elementCount);
    } else if (bitsPerFloat === 8) {
      this._quantizedData = new Uint8Array(elementCount);
    } else {
      console.log('Bad bitsPerFloat ' + bitsPerFloat);
    }
   } else {
    if (bitsPerFloat === 32) {
      this._data = data;
    } else if (bitsPerFloat === 16) {
      this._quantizedData = new Uint16Array(data);
    } else if (bitsPerFloat === 8) {
      this._quantizedData = new Uint8Array(data);
    } else {
      console.log('Bad bitsPerFloat ' + bitsPerFloat);
    }
  }
  this._name = 'None';
};
window['Buffer'] = Buffer;
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
  var result;
  if (this._bitsPerFloat === 32) {
    result = new Buffer(this._dims, this._data);
  } else {
    result = new Buffer(this._dims, this._quantizedData, {
      bitsPerFloat: this._bitsPerFloat,
      min: this._min,
      max: this._max});
  }
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
Buffer.prototype.areAllClose = function(b, tolerance) {
  if (_.isUndefined(tolerance)) {
    tolerance = 0.000001;
  }
  var a = this;
  if (!a) {
    console.log('Buffer a is empty or undefined');
    return false;
  }

  if (!b) {
    console.log('Buffer b is empty or undefined');
    return false;
  }

  if (a._dims._dims.length != b._dims._dims.length) {
    console.log('Buffers have different numbers of dimensions - ' + a + ' vs ' + b);
    return false;
  }

  if (!a._dims.areEqualTo(b._dims)) {
    console.log('Buffers are different sizes - ' + a + ' vs ' + b);
    return false;
  }

  if (a._data.length !== a._dims.elementCount()) {
    console.log('The length of a\'s data buffer doesn\'t match the required number of elements - a._data.length = ' + a._data.length + ', a._dims.elementCount() = ' + a._dims.elementCount() );
    return false;
  }

  if (b._data.length !== b._dims.elementCount()) {
    console.log('The length of b\'s data buffer doesn\'t match the required number of elements - b._data.length = ' + b._data.length + ', b._dims.elementCount() = ' + b._dims.elementCount() );
    return false;
  }

  var differentCount = 0.0;
  var totalDelta = 0.0;
  var aData = a._data;
  var bData = b._data;
  var offset = 0;
  var aNaNCount = 0;
  var elementCount = a._dims.elementCount();
  while (offset < elementCount) {
    var aValue = aData[offset];
    if (isNaN(aValue)) {
      aNaNCount += 1;
    }
    var bValue = bData[offset];
    if (isNaN(bValue)) {
      bNaNCount += 1;
    }
    var delta = (aValue - bValue);
    var absDelta = Math.abs(delta);
    if (absDelta > tolerance) {
      differentCount += 1;
    }
    totalDelta += absDelta;
    offset += 1;
  }

  var differentPercentage = (100 * (differentCount / elementCount));
  var meanDelta = (totalDelta / elementCount);
  console.log('Buffers contained ' +
  differentPercentage + '% different values' +
  ' (' + differentCount + ')' +
  ' mean delta = ' + meanDelta +
  ' ' + a + ' vs' +
  ' ' + b);
  if (differentCount > 0) {
    return false;
  }

  return true;
};
Buffer.prototype.extractSubregion = function(originY, originX, size) {
  var inputDims = this._dims._dims;

  var inputWidth = inputDims[1];
  var inputChannels = inputDims[2];

  var regionWidth = size._dims[1];
  var regionChannels = size._dims[2];

  var output;
  if (this._bitsPerFloat === 32) {
    output = new Buffer(size);
  } else {
    output = new Buffer(size, undefined, {
      bitsPerFloat: this._bitsPerFloat,
      min: this._min,
      max: this._max});
  }
  output._name = this._name + ' subregion';

  var elementsPerInputRow = (inputWidth * inputChannels);
  var elementsPerRegionRow = (regionWidth * regionChannels);

  var inputData;
  var regionData;
  if (this._bitsPerFloat === 32) {
    inputData = this._data;
    regionData = output._data;
  } else {
    inputData = this._quantizedData;
    regionData = output._quantizedData;
  }
  var inputOffset = this._dims.offset(originY, originX, 0);
  var regionOffset = 0;
  var elementCount = size.elementCount();
  while (regionOffset < elementCount) {
    var inputRow = inputData.subarray(inputOffset, (inputOffset + elementsPerRegionRow));
    var regionRow = regionData.subarray(regionOffset, (regionOffset + elementsPerRegionRow));
    regionRow.set(inputRow);
    inputOffset += elementsPerInputRow;
    regionOffset += elementsPerRegionRow;
  }

  return output;
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
Buffer.prototype.getGPUBuffer = function(gpuCalculator, inputDims) {
  if (_.isUndefined(inputDims)) {
    inputDims = this._dims;
  }
  if (_.isUndefined(this._gpuBuffer)) {
    var data;
    if (this._bitsPerFloat === 32) {
      data = this._data;
    } else {
      data = this._quantizedData;
    }
    var dims = inputDims._dims;
    var width = dims[1];
    var height = dims[0];
    var channels;
    if (dims.length > 2) {
      channels = dims[2];
    } else {
      channels = 1;
    }

    console.assert(this._name !== 'None');

    var gpuBuffer = gpuCalculator.createBuffer({
      width: width,
      height: height,
      channels: channels,
      bitDepth: this._bitsPerFloat,
      data: data,
      debugInfo: this.toString()});
    this._gpuBuffer = gpuBuffer;
  }
  return this._gpuBuffer;
};
function bufferFromTagDict(tagDict) {
  console.assert(tagDict.type === JP_DICT);
  var bitsPerFloat = tagDict.getUintFromDict('float_bits');
  console.assert((bitsPerFloat === 32) || (bitsPerFloat === 16) || (bitsPerFloat === 8));
  var dimsTag = tagDict.getTagFromDict('dims');
  var dimsSubTags = dimsTag.getSubTags();
  var dimsValues = [];
  _.each(dimsSubTags, function(subTag) {
    console.assert(subTag.type === JP_UINT);
    dimsValues.push(subTag.value);
  });
  var dims = new Dimensions(dimsValues);
  var elementCount = dims.elementCount();

  if (bitsPerFloat === 32) {
    var dataTag = tagDict.getTagFromDict("data");
    console.assert(dataTag.type === JP_FARY);
    console.assert(dataTag.length === (elementCount * 4));

    var dataTagArray = dataTag.value;
    var buffer = new Buffer(dims, dataTagArray);
  } else if ((bitsPerFloat === 16) || (bitsPerFloat == 8)) {
    var dataTag = tagDict.getTagFromDict("quantized_data");
    console.assert(dataTag.type === JP_BLOB);
    var bytesPerElement = (bitsPerFloat / 8);
    var expectedByteCount = (Math.floor(((elementCount * bytesPerElement) + 3) / 4) * 4);
    console.assert(dataTag.length === expectedByteCount);

    var min = tagDict.getFloatFromDict('min');
    var max = tagDict.getFloatFromDict('max');
    if (false) {
      var intRange = (1 << bitsPerFloat);
      var spread = ((max - min) / intRange);

      var floatBuffer = new Float32Array(elementCount);
      var quantizedBuffer;
      if (bitsPerFloat === 16) {
        quantizedBuffer = new Uint16Array(dataTag.value);
      } else if (bitsPerFloat === 8) {
        quantizedBuffer = new Uint8Array(dataTag.value);
      } else {
        console.log('Bad bitsPerFloat ' + bitsPerFloat);
        return null;
      }
      for (var index = 0; index < elementCount; index += 1) {
        var quantizedValue = quantizedBuffer[index];
        floatBuffer[index] = ((quantizedValue * spread) + min);
      }
      var buffer = new Buffer(dims, floatBuffer, {bitsPerFloat: 32, min: 0, max: 1});
    } else {
      var buffer = new Buffer(dims, dataTag.value, {bitsPerFloat: bitsPerFloat, min: min, max: max});
    }
  }
  return buffer;
}
// Warning - only use this function for debugging, since it's synchronous
function bufferFromFileAtURL(url) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.responseType = 'arraybuffer';
  var isDone = false;
  xhr.onload = function(e) {
    isDone = true;
    console.log('loaded');
  };
  xhr.onerror = function(e) {
    alert("Error " + e.target.status + " occurred while receiving the document.");
    isDone = true;
  };
  xhr.send();
  // Here be dragons! Faking a synchronous XHR call, since we need the
  // responseType, and that's not supported with modern sync requests.
  while (!isDone) {
    // The flag for this should be set in the callbacks.
  }
  var tag = tagFromMemory(xhr.response, 0);
  var buffer = bufferFromTagDict(tag);
  buffer.setName(url);
  return buffer;
}

function delayedBufferFromFileAtURL(url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.responseType = 'arraybuffer';
  xhr.onload = function(e) {
    var tag = tagFromMemory(xhr.response, 0);
    var buffer = bufferFromTagDict(tag);
    buffer.setName(url);
    callback(buffer)
  };
  xhr.onerror = function(e) {
    alert("Error " + e.target.status + " occurred while receiving the document.");
  };
  xhr.send();
}

/**
 * @constructor
 */
Network = function(filename, onLoad, options) {
  if (_.isUndefined(options)) {
    options = {};
  }
  this._isLoaded = false;
  this._isHomebrewed = true;
  this._fileTag = null;
  //this._testResults = true;
  //this._runOnlyLayer = 20;
  //this._doProfile = true;
  this._onLoad = onLoad;
  var xhr = new XMLHttpRequest();
  xhr.open('GET', filename, true);
  xhr.responseType = 'arraybuffer';
  xhr.onload = function (myThis) {
    var myNetwork = myThis;
    return function(e) {
      if (this.status == 200) {
        myNetwork.initializeFromArrayBuffer(this.response);
      }
    };
  }(this);
  xhr.onerror = function(e) {
    alert("Error " + e.target.status + " occurred while receiving the document.");
  };
  if (options.progress) {
    this.onprogress = options.progress;
    xhr.onprogress = function (myThis) {
      return function(event) {
        var total;
        if (event.lengthComputable) {
          total = event.total;
        } else {
          total = 64140288; // Known length of our network file, and a good guess for similar ones
        }
        var percentComplete = (event.loaded / total) * 100;
        myThis.onprogress(percentComplete);
      };
    }(this);
  }
  xhr.send();
};
window['Network'] = Network;
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
  var previousClassName;
  _.each(layerSubTags, function(layerSubTag) {
    var layerNode = nodeFromTag(layerSubTag);

    var className = layerNode._className;
    var shouldSkip = ((className === 'relu') && (previousClassName === 'relu'));
    previousClassName = className;
    if (shouldSkip) {
      return;
    }
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

  if (graphDict.getTagFromDict('copyright')) {
    console.log('Neural network parameters ' + graphDict.getStringFromDict('copyright'));
  }

  this._onLoad(this);
};
Network.prototype.run = function(input, layerOffset) {
  if (_.isUndefined(layerOffset)) {
    layerOffset = 0;
  }
  var filePath = 'data/c_dog_blobs/';
  var currentInput = input;
  var howManyLayers = (this._layers.length + layerOffset);
  for (var index = 0; index < howManyLayers; index += 1) {
    if (!_.isUndefined(this._runOnlyLayer) && (index != this._runOnlyLayer)) {
      continue;
    }
    var layer = this._layers[index];

    if (this._testResults) {
      var inputIndexString = ('000' + ((index * 2) + 1)).slice(-3);
      var expectedInputFilename = filePath + inputIndexString + '_input.blob';
      var expectedInput = bufferFromFileAtURL(expectedInputFilename);
      if (!expectedInput.areAllClose(currentInput)) {
        console.log('inputs differ for ' + index + ' - ' + layer.constructor.name);
        currentInput = expectedInput;
      } else {
        console.log('inputs are equal for ' + index);
      }
    }

    if (this._doProfile) {
      console.log('Running ' + layer.constructor.name)
      var startTime = new Date().getTime();
    }
    var currentOutput = layer.run(currentInput);
    if (this._doProfile) {
      var endTime = new Date().getTime();
      var duration = (endTime - startTime);
      console.log(layer._name + ' took ' + duration + ' ms');
    }
    currentOutput.setName(layer.constructor.name + ' output');
//    console.log('currentOutput = ' + currentOutput);

    if (this._testResults) {
      var outputIndexString = ('000' + ((index * 2) + 1)).slice(-3);
      var expectedOutputFilename = filePath + outputIndexString + '_output.blob';
      var expectedOutput = bufferFromFileAtURL(expectedOutputFilename);
      if (!expectedOutput.areAllClose(currentOutput)) {
        console.log('outputs differ for ' + index + ' - ' + layer.constructor.name);
        currentOutput = expectedOutput;
      } else {
        console.log('outputs are equal for ' + index);
      }
    }

    if (!_.isUndefined(this._runOnlyLayer)) {
      return null;
    }

    currentInput = currentOutput;
  }
  return currentInput;
};

/**
 * @constructor
 */
BinaryFormat = function(arrayBuffer) {
  this.arrayBuffer = arrayBuffer;
  this.cursor = 0;
};
window['BinaryFormat'] = BinaryFormat;
JP_CHAR = 0x52414843; // 'CHAR'
JP_UINT = 0x544E4955; // 'UINT'
JP_FL32 = 0x32334C46; // 'FL32'
JP_FARY = 0x59524146; // 'FARY'
JP_DICT = 0x54434944; // 'DICT'
JP_LIST = 0x5453494C; // 'LIST'
JP_BLOB = 0x424F4C42; // 'BLOB'
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

/**
 * @constructor
 */
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
  } else if (type === JP_BLOB) {
    value = valuesBuffer;
  } else {
    console.log('Unknown type ' + type);
    return null;
  }
  this.type = type;
  this.length = length;
  this.value = value;
};
window['BinaryTag'] = BinaryTag;
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
    'max': MaxNode
  };
  var tagClass = tag.getStringFromDict('class');
  var jsClass = classLookup[tagClass];
  var result = new jsClass(tag);
  var tagName = tag.getStringFromDict('name');
  result._className = tagClass;
  result._name = tagName;
  return result;
}

/**
 * @constructor
 */
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
window['ConvNode'] = ConvNode;
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

/**
 * @constructor
 */
function DropoutNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'dropout', 'Wrong class name in tag');
}
window['DropoutNode'] = DropoutNode;
DropoutNode.prototype.run = function(input) {
  return input;
};

/**
 * @constructor
 */
function FlatNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'flat', 'Wrong class name in tag');
}
window['FlatNode'] = FlatNode;
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

/**
 * @constructor
 */
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
window['GConvNode'] = GConvNode;
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

/**
 * @constructor
 */
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
window['NeuronNode'] = NeuronNode;
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

/**
 * @constructor
 */
function NormalizeNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'normalize', 'Wrong class name in tag');

  this._windowSize = tag.getUintFromDict('size');
  this._k = tag.getFloatFromDict('k');
  this._alpha = tag.getFloatFromDict('alpha');
  this._beta = tag.getFloatFromDict('beta');
}
window['NormalizeNode'] = NormalizeNode;
NormalizeNode.prototype.run = function(input) {
  this._output = matrixLocalResponse(input, this._windowSize, this._k, this._alpha, this._beta);
  return this._output;
};

/**
 * @constructor
 */
function PoolNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'pool', 'Wrong class name in tag');

  this._patchWidth = tag.getUintFromDict('psize');
  this._stride = tag.getUintFromDict('stride');
  this._mode = tag.getStringFromDict('mode');
}
window['PoolNode'] = PoolNode;
PoolNode.prototype.run = function(input) {
  this._output = matrixMaxPatch(input, this._patchWidth, this._stride);
  return this._output;
};

/**
 * @constructor
 */
function ReluNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'relu', 'Wrong class name in tag');
}
window['ReluNode'] = ReluNode;
ReluNode.prototype.run = function(input) {
  this._output = matrixMax(input, 0.0);
  return this._output;
};

/**
 * @constructor
 */
function MaxNode(tag) {
  var className = tag.getStringFromDict('class');
  console.assert(className === 'max', 'Wrong class name in tag');
}
window['MaxNode'] = MaxNode;
MaxNode.prototype.run = function(input) {
  this._output = matrixSoftmax(input);
  return this._output;
};

/**
 * @constructor
 */
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
window['PrepareInputNode'] = PrepareInputNode;
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

  if (kernels._bitsPerFloat === 32) {
    output = matrixGemm(
      m,
      n,
      k,
      alpha,
      kernels,
      lda,
      patches,
      ldb,
      beta,
      output,
      ldc
    );
  } else {
    output = matrixGemmScaleA(
      m,
      n,
      k,
      alpha,
      kernels,
      lda,
      patches,
      ldb,
      beta,
      output,
      ldc
    );
  }
  output.reshape(outputDims);

  return output;
}

function matrixGemm(
  m,
  n,
  k,
  alpha,
  aBuffer,
  lda,
  bBuffer,
  ldb,
  beta,
  cBuffer,
  ldc) {

  var useNaive = false;
  if (useNaive) {
    return naiveGemm(
      m,
      n,
      k,
      alpha,
      aBuffer._data,
      lda,
      bBuffer._data,
      ldb,
      beta,
      cBuffer._data,
      ldc
    );
  } else {
    return glGemm(
      m,
      n,
      k,
      alpha,
      aBuffer,
      lda,
      bBuffer,
      ldb,
      beta,
      cBuffer,
      ldc
    );
  }

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

  var outputDims = new Dimensions(n, m);
  var outputBuffer = new Buffer(outputDims, c);

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

  return outputBuffer;
}

function matrixGemmScaleA(
  m,
  n,
  k,
  alpha,
  aBuffer,
  lda,
  bBuffer,
  ldb,
  beta,
  cBuffer,
  ldc,
  aScale,
  aOffset,
  aBitDepth) {

  var useNaive = false;
  if (useNaive) {
    return naiveGemmScaleA(
      m,
      n,
      k,
      alpha,
      aBuffer._quantizedData,
      lda,
      bBuffer._data,
      ldb,
      beta,
      cBuffer._data,
      ldc,
      aScale,
      aOffset
    );
  } else {
    return glGemm(
      m,
      n,
      k,
      alpha,
      aBuffer,
      lda,
      bBuffer,
      ldb,
      beta,
      cBuffer,
      ldc,
      aBuffer._spread,
      aBuffer._min,
      aBuffer._bitsPerFloat
    );
  }

}

function naiveGemmScaleA(
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
  ldc,
  aScale,
  aOffset) {

  var outputDims = new Dimensions(n, m);
  var outputBuffer = new Buffer(outputDims, c);

  for (var i = 0; i < m; i++) {
    for (var j = 0; j < n; j++) {
      var total = 0.0;
      for (var l = 0; l < k; l++) {
        var aIndex = ((lda * l) + i);
        var aValue = ((a[aIndex] * aScale) + aOffset);
        var bIndex = ((ldb * j) + l);
        var bValue = b[bIndex];
        total += (aValue * bValue);
      }
      var cIndex = ((ldc * j) + i);
      var oldCValue = c[cIndex];
      c[cIndex] = ((alpha * total) + (beta * oldCValue));
    }
  }

  return outputBuffer;
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
  var n = inputDims._dims[0];
  var k = inputDims._dims[1];
  var alpha = 1.0;
  var lda = m;
  var ldb = k;
  var ldc = m;
  var beta = 0.0;

  if (weights._bitsPerFloat === 32) {
    output = matrixGemm(
      m,
      n,
      k,
      alpha,
      weights,
      lda,
      input,
      ldb,
      beta,
      output,
      ldc
    );
  } else {
    output = matrixGemmScaleA(
      m,
      n,
      k,
      alpha,
      weights,
      lda,
      input,
      ldb,
      beta,
      output,
      ldc,
      weights._spread,
      weights._min,
      weights._bitsPerFloat
    );
  }
  output.reshape(outputDims);

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
  var prereadCount = Math.floor((windowSize / 2) - 0);

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
      var rightIndex = (channel + Math.floor(windowSize / 2));
      if (rightIndex < inputChannels) {
        averagedScale += magBufferData[rightIndex];
      }
      magnitudeData[magnitudeOffset + channel] = (averagedScale + k);
      var leftIndex = (channel - Math.floor(windowSize / 2));
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
  var useNaive = false;
  var output;
  if (useNaive) {
    output = naiveMaxPatch(input, patchWidth, stride);
  } else {
    output = glMaxPatch(input, patchWidth, stride);
  }
  return output;
}

function naiveMaxPatch(input, patchWidth, stride) {

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
  var useNaive = true;
  if (useNaive) {
    return naiveMax(input, maxValue);
  } else {
    return glMax(input, maxValue);
  }
}

function naiveMax(input, maxValue) {
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

var g_gpuCalculator;
function getGPUCalculator() {
  if (_.isUndefined(g_gpuCalculator)) {
    g_gpuCalculator = new GPUCalculator();
  }
  return g_gpuCalculator;
}

var gemmShader = "                                              \n\
  precision mediump float;                                      \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aScale;                                          \n\
  uniform sampler2D b;                                          \n\
  uniform vec2 bScale;                                          \n\
  uniform sampler2D c;                                          \n\
  uniform vec2 cScale;                                          \n\
  uniform float alpha;                                          \n\
  uniform float beta;                                           \n\
  uniform float k;                                              \n\
  uniform float aValueScale;                                    \n\
  uniform float aValueOffset;                                   \n\
  void main(void) {                                             \n\
    vec2 texCoord = outTexCoord;                                \n\
    float i = texCoord.x;                                       \n\
    float j = texCoord.y;                                       \n\
    vec2 cCoords = vec2(i, j) * cScale;                         \n\
    float cValue;                                               \n\
    if (beta != 0.0) {                                          \n\
      cValue = texture2D(c, cCoords).r;                         \n\
    } else {                                                    \n\
      cValue = 0.0;                                             \n\
    }                                                           \n\
    float total = 0.0;                                          \n\
    for (int l = 0; l < 10000; l += 1) {                        \n\
      if (l >= int(k)) {                                        \n\
        break;                                                  \n\
      }                                                         \n\
      float lCoord = (float(l) + 0.5);                          \n\
      vec2 aCoords = vec2(i, lCoord) * aScale;                  \n\
      float aValue = texture2D(a, aCoords).r;                   \n\
      aValue = ((aValue * aValueScale) + aValueOffset);         \n\
      vec2 bCoords = vec2(lCoord, j) * bScale;                  \n\
      float bValue = texture2D(b, bCoords).r;                   \n\
      total += (aValue * bValue);                               \n\
    }                                                           \n\
    gl_FragColor.r = (alpha * total) + (beta * cValue);         \n\
  }                                                             \n\
";

var gemmShader4x = "                                            \n\
  precision mediump float;                                      \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aScale;                                          \n\
  uniform sampler2D b;                                          \n\
  uniform vec2 bScale;                                          \n\
  uniform sampler2D c;                                          \n\
  uniform vec2 cScale;                                          \n\
  uniform float alpha;                                          \n\
  uniform float beta;                                           \n\
  uniform float k;                                              \n\
  uniform float aValueScale;                                    \n\
  uniform float aValueOffset;                                   \n\
  void main(void) {                                             \n\
    vec2 texCoord = outTexCoord;                                \n\
    float startI = ((texCoord.x - 0.5) * 4.0) + 0.5;            \n\
    float j = texCoord.y;                                       \n\
    vec2 cCoords = vec2(texCoord.x, texCoord.y) * cScale;       \n\
    vec4 cPixel = texture2D(c, cCoords);                        \n\
    for (int iInc = 0; iInc < 4; iInc += 1) {                   \n\
      float i = startI + float(iInc);                           \n\
      float iCoord = float(int(i) / 4) + 0.5;                   \n\
      float cValue;                                             \n\
      if (iInc == 0) {                                          \n\
        cValue = cPixel.x;                                      \n\
      } else if (iInc == 1) {                                   \n\
        cValue = cPixel.y;                                      \n\
      } else if (iInc == 2) {                                   \n\
        cValue = cPixel.z;                                      \n\
      } else if (iInc == 3) {                                   \n\
        cValue = cPixel.w;                                      \n\
      }                                                         \n\
      float total;                                              \n\
      if (beta != 0.0) {                                        \n\
        total = (cValue * beta);                                \n\
      } else {                                                  \n\
        total = 0.0;                                            \n\
      }                                                         \n\
      for (int l = 0; l < 10000; l += 1) {                      \n\
        if (l >= int(k)) {                                      \n\
          break;                                                \n\
        }                                                       \n\
        float aLCoord = float(l) + 0.5;                         \n\
        vec2 aCoords = vec2(iCoord, aLCoord) * aScale;          \n\
        vec4 aPixel = texture2D(a, aCoords);                    \n\
        float aValue;                                           \n\
        if (iInc == 0) {                                        \n\
          aValue = aPixel.x;                                    \n\
        } else if (iInc == 1) {                                 \n\
          aValue = aPixel.y;                                    \n\
        } else if (iInc == 2) {                                 \n\
          aValue = aPixel.z;                                    \n\
        } else if (iInc == 3) {                                 \n\
          aValue = aPixel.w;                                    \n\
        }                                                       \n\
        aValue = ((aValue * aValueScale) + aValueOffset);       \n\
        float bLCoord = float(l / 4) + 0.5;                     \n\
        int bLComponent = int(mod(float(l), 4.0));              \n\
        vec2 bCoords = vec2(bLCoord, j) * bScale;               \n\
        vec4 bPixel = texture2D(b, bCoords);                    \n\
        float bValue;                                           \n\
        if (bLComponent == 0) {                                 \n\
          bValue = bPixel.x;                                    \n\
        } else if (bLComponent == 1) {                          \n\
          bValue = bPixel.y;                                    \n\
        } else if (bLComponent == 2) {                          \n\
          bValue = bPixel.z;                                    \n\
        } else if (bLComponent == 3) {                          \n\
          bValue = bPixel.w;                                    \n\
        }                                                       \n\
        total += (aValue * bValue);                             \n\
      }                                                         \n\
      float result = (alpha * total);                           \n\
      if (iInc == 0) {                                          \n\
        gl_FragColor.r = result;                                \n\
      } else if (iInc == 1) {                                   \n\
        gl_FragColor.g = result;                                \n\
      } else if (iInc == 2) {                                   \n\
        gl_FragColor.b = result;                                \n\
      } else if (iInc == 3) {                                   \n\
        gl_FragColor.a = result;                                \n\
      }                                                         \n\
    }                                                           \n\
  }                                                             \n\
";

function glGemm(
  m,
  n,
  inputK,
  alpha,
  inputA,
  lda,
  inputB,
  ldb,
  inputBeta,
  inputC,
  ldc,
  inputAScale,
  aOffset,
  aBitDepth) {

  if (_.isUndefined(inputAScale)) {
    inputAScale = 1.0;
  }
  if (_.isUndefined(aOffset)) {
    aOffset = 0.0;
  }
  if (_.isUndefined(aBitDepth)) {
    aBitDepth = 32;
  }

  var aScale;
  if ((aBitDepth === 32) || (aBitDepth === 16)) {
    aScale = inputAScale;
  } else {
    var levels = ((1 << aBitDepth) - 1);
    aScale = (inputAScale * levels);
  }

  var gpuCalculator = getGPUCalculator();

  var maxTextureSize = 4096;

  var previousCGPUBuffer = null;

  var use4x = (((m % 4) == 0) && ((inputK % 4) == 0));
  var shaderText;
  var aFullDims;
  var bFullDims;
  var cDims;
  if (use4x) {
    shaderText = gemmShader4x;
    aFullDims = new Dimensions(inputK, (m / 4), 4);
    bFullDims = new Dimensions(n, (inputK / 4), 4);
    cDims = new Dimensions(n, (m / 4), 4);
  } else {
    shaderText = gemmShader;
    aFullDims = new Dimensions(inputK, m, 1);
    bFullDims = new Dimensions(n, inputK, 1);
    cDims = new Dimensions(n, m, 1);
  }

  var aFullBuffer = inputA.view().reshape(aFullDims);
  var bFullBuffer = inputB.view().reshape(bFullDims);

  var kStepCount = Math.ceil(inputK / maxTextureSize);
  for (var kStep = 0; kStep < kStepCount; kStep += 1) {
    var currentK = (inputK - (kStep * maxTextureSize));
    if (currentK > maxTextureSize) {
      currentK = maxTextureSize;
    }
    var originK = (kStep * maxTextureSize);

    var aDims;
    var bDims;
    if (use4x) {
      aDims = new Dimensions(currentK, (m / 4), 4);
      bDims = new Dimensions(n, (currentK / 4), 4);
    } else {
      aDims = new Dimensions(currentK, m, 1);
      bDims = new Dimensions(n, currentK, 1);
    }

    var aGPUBuffer;
    var bGPUBuffer;
    if (kStep === 0) {
      inputA.setName('glGemm() inputA');
      aGPUBuffer = inputA.getGPUBuffer(gpuCalculator, aDims);
      inputB.setName('glGemm() inputB kStep=' + kStep);
      bGPUBuffer = inputB.getGPUBuffer(gpuCalculator, bDims);
    } else {
      aFullBuffer.setName('glGemm() aFullBuffer');
      var aSubregionBuffer = aFullBuffer.extractSubregion(originK, 0, aDims);
      bFullBuffer.setName('glGemm() bFullBuffer');
      var bSubregionBuffer;
      if (use4x) {
        bSubregionBuffer = bFullBuffer.extractSubregion(0, (originK / 4), bDims);
      } else {
        bSubregionBuffer = bFullBuffer.extractSubregion(0, originK, bDims);
      }
      aGPUBuffer = aSubregionBuffer.getGPUBuffer(gpuCalculator, aDims);
      bGPUBuffer = bSubregionBuffer.getGPUBuffer(gpuCalculator, bDims);
    }

    var beta;
    if (kStep === 0) {
      var previousCBuffer;
      if (inputBeta > 0.0) {
        previousCBuffer = new Buffer(cDims, c._data);
      } else {
        previousCBuffer = new Buffer(cDims, null);
      }
      previousCBuffer.setName('glGemm() previousCBuffer');
      previousCGPUBuffer = previousCBuffer.getGPUBuffer(gpuCalculator);
      beta = inputBeta;
    } else {
      beta = 1.0;
    }

    var uniforms = {
      'alpha': alpha,
      'beta': beta,
      'k': currentK,
      'aValueScale': aScale,
      'aValueOffset': aOffset
    };
    var inputBuffers = {
      'a': aGPUBuffer,
      'b': bGPUBuffer,
      'c': previousCGPUBuffer
    };

    var startTime = new Date().getTime();
    var outputCGPUBuffer = gpuCalculator.applyShader({
      shaderText: shaderText,
      inputBuffers: inputBuffers,
      uniforms: uniforms,
      width: cDims._dims[1],
      height: cDims._dims[0]
    });

    if (kStep !== 0) {
      gpuCalculator.deleteBuffer(aGPUBuffer);
    }
    gpuCalculator.deleteBuffer(bGPUBuffer);
    if (previousCGPUBuffer) {
      gpuCalculator.deleteBuffer(previousCGPUBuffer);
    }
    previousCGPUBuffer = outputCGPUBuffer;
  }

  var outputChannels = cDims._dims[2];
  var outputData = gpuCalculator.getResult(previousCGPUBuffer, outputChannels);
  var endTime = new Date().getTime();
//  console.log('gemm took ' + (endTime - startTime) + 'ms');

//  gpuCalculator.deleteBuffer(aBuffer);
  gpuCalculator.deleteBuffer(previousCGPUBuffer);

  var outputCDims = new Dimensions(n, m);
  var outputBuffer = new Buffer(outputCDims, outputData);

  return outputBuffer;
}

var maxPatchShader = "                     \n\
  precision mediump float;                                      \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aScale;                                          \n\
  uniform float patchWidth;                                     \n\
  uniform float stride;                                         \n\
  uniform float channelCount;                                   \n\
  void main(void) {                                             \n\
    vec2 texCoord = outTexCoord;                                \n\
    float outputXAndChannels = floor(texCoord.x);               \n\
    float outputChannel = mod(outputXAndChannels, channelCount); \n\
    float outputX = floor(outputXAndChannels / channelCount);   \n\
    float outputY = floor(texCoord.y);                          \n\
    float inputOriginX = (outputX * stride);                    \n\
    float inputOriginY = (outputY * stride);                    \n\
    vec4 patchMax = vec4(-100000000.0, -100000000.0, -100000000.0, -100000000.0); \n\
    for (int patchY = 0; patchY < 100; patchY += 1) {           \n\
      if (patchY >= int(patchWidth)) {                               \n\
        break;                                                  \n\
      }                                                         \n\
      float inputY = ((inputOriginY + float(patchY)) + 0.5);    \n\
      for (int patchX = 0; patchX < 100; patchX += 1) {         \n\
        if (patchX >= int(patchWidth)) {                        \n\
          break;                                                \n\
        }                                                       \n\
        float inputX = ((inputOriginX + float(patchX)));        \n\
        float inputXAndChannel = (inputX * channelCount) + outputChannel; \n\
        vec2 inputCoords = vec2(inputXAndChannel + 0.5, inputY) * aScale; \n\
        vec4 inputPixel = texture2D(a, inputCoords);            \n\
        patchMax = max(patchMax, inputPixel);                   \n\
      }                                                         \n\
    }                                                           \n\
    gl_FragColor = patchMax;                                    \n\
  }                                                             \n\
";

function glMaxPatch(input, patchWidth, stride) {
  var gpuCalculator = getGPUCalculator();

  var inputDims = input._dims._dims;
  var imageCount = inputDims[0];
  console.assert((imageCount === 1), 'Only handles the single-image case');
  var inputWidth = inputDims[2];
  var inputHeight = inputDims[1];
  var inputChannels = inputDims[3];

  var quantizedChannels = Math.floor(inputChannels / 4);
  console.assert(((quantizedChannels * 4) === inputChannels), 'Channel count must be a multiple of four');
  var inputGPUWidth = (inputWidth * quantizedChannels);

  var inputGPUDims = new Dimensions(inputHeight, inputGPUWidth, 4);

  var outputWidth = Math.round(Math.floor((inputWidth - patchWidth) / stride) + 1);
  var outputHeight = Math.round(Math.floor((inputHeight - patchWidth) / stride) + 1);
  var outputChannels = inputChannels;
  var outputDims = new Dimensions(imageCount, outputHeight, outputWidth, outputChannels);

  var outputGPUWidth = (outputWidth * quantizedChannels);
  var outputGPUDims = new Dimensions(outputHeight, outputGPUWidth, 4);

  var inputGPUBuffer = input.getGPUBuffer(gpuCalculator, inputGPUDims);
  var uniforms = {
    'patchWidth': patchWidth,
    'stride': stride,
    'channelCount': quantizedChannels
  };
  var inputBuffers = {
    'a': inputGPUBuffer
  };

  var outputGPUBuffer = gpuCalculator.applyShader({
    shaderText: maxPatchShader,
    inputBuffers: inputBuffers,
    uniforms: uniforms,
    width: outputGPUWidth,
    height: outputHeight
  });

  var outputData = gpuCalculator.getResult(outputGPUBuffer, 4);
  gpuCalculator.deleteBuffer(inputGPUBuffer);
  gpuCalculator.deleteBuffer(outputGPUBuffer);

  var outputBuffer = new Buffer(outputGPUDims, outputData);
  outputBuffer.reshape(outputDims);

  return outputBuffer;
}

var maxShader = "                     \n\
  precision mediump float;                                      \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aScale;                                          \n\
  uniform float maxValue;                                       \n\
  void main(void) {                                             \n\
    vec2 texCoord = outTexCoord;                                \n\
    vec2 inputCoords = (texCoord * aScale);                     \n\
    vec4 inputPixel = texture2D(a, inputCoords);                \n\
    gl_FragColor = max(inputPixel, vec4(maxValue, maxValue, maxValue, maxValue)); \n\
  }                                                             \n\
";

function glMax(input, maxValue) {
  var gpuCalculator = getGPUCalculator();

  var inputDims = input._dims._dims;
  var imageCount = inputDims[0];
  console.assert((imageCount === 1), 'Only handles the single-image case');
  var inputHeight = inputDims[1];
  var inputWidth;
  if (inputDims.length < 3) {
    inputWidth = 1;
  } else {
    inputWidth = inputDims[2];
  }
  var inputChannels;
  if (inputDims.length < 4) {
    inputChannels = 1;
  } else {
    inputChannels = inputDims[3];
  }

  var quantizedChannels = (inputChannels / 4);
  if (inputWidth === 1) {
    inputWidth = inputHeight;
    inputHeight = 1;
  }
  var inputGPUWidth = (inputWidth * quantizedChannels);

  var inputGPUDims = new Dimensions(inputHeight, inputGPUWidth, 4);

  var inputGPUBuffer = input.getGPUBuffer(gpuCalculator, inputGPUDims);
  var uniforms = {
    'maxValue': maxValue
  };
  var inputBuffers = {
    'a': inputGPUBuffer
  };

  var outputGPUBuffer = gpuCalculator.applyShader({
    shaderText: maxShader,
    inputBuffers: inputBuffers,
    uniforms: uniforms,
    width: inputGPUWidth,
    height: inputHeight
  });

  var outputData = gpuCalculator.getResult(outputGPUBuffer, 4);
  gpuCalculator.deleteBuffer(inputGPUBuffer);
  gpuCalculator.deleteBuffer(outputGPUBuffer);

  var outputBuffer = new Buffer(inputGPUDims, outputData);
  outputBuffer.reshape(input._dims);

  return outputBuffer;
}

// Shim for Internet Explorer's missing slice(), see
// http://stackoverflow.com/questions/21440050/arraybuffer-prototype-slice-shim-for-ie
if (!ArrayBuffer.prototype.slice) {
  //Returns a new ArrayBuffer whose contents are a copy of this ArrayBuffer's
  //bytes from `begin`, inclusive, up to `end`, exclusive
  ArrayBuffer.prototype.slice = function (begin, end) {
    //If `begin` is unspecified, Chrome assumes 0, so we do the same
    if (begin === void 0) {
      begin = 0;
    }

    //If `end` is unspecified, the new ArrayBuffer contains all
    //bytes from `begin` to the end of this ArrayBuffer.
    if (end === void 0) {
      end = this.byteLength;
    }

    //Chrome converts the values to integers via flooring
    begin = Math.floor(begin);
    end = Math.floor(end);

    //If either `begin` or `end` is negative, it refers to an
    //index from the end of the array, as opposed to from the beginning.
    if (begin < 0) {
      begin += this.byteLength;
    }
    if (end < 0) {
      end += this.byteLength;
    }

    //The range specified by the `begin` and `end` values is clamped to the 
    //valid index range for the current array.
    begin = Math.min(Math.max(0, begin), this.byteLength);
    end = Math.min(Math.max(0, end), this.byteLength);

    //If the computed length of the new ArrayBuffer would be negative, it 
    //is clamped to zero.
    if (end - begin <= 0) {
      return new ArrayBuffer(0);
    }

    var result = new ArrayBuffer(end - begin);
    var resultBytes = new Uint8Array(result);
    var sourceBytes = new Uint8Array(this, begin, end - begin);

    resultBytes.set(sourceBytes);

    return result;
  };
}

