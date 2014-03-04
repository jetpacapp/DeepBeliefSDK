// Copyright 2014 Jetpac Inc
// All rights reserved.
// Pete Warden <pete@jetpac.com>

Dimensions = function(input) {
  if (input instanceof Array) {
    this._dims = input;
  } else if (input instanceof Dimensions) {
    this._dims = input._dims;
  } else {
    throw "Unknown input type to Dimensions() - " + input;
  }
};
Dimensions.prototype.elementCount = function() {
  var result = 1;
  for (var index = 0; index < this._dims.length; index += 1) {
    result *= this._dims[index];
  }
  return result;
};
Dimensions.prototype.offset = function(indices) {
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
  return '(' + _dims.join(', ') + ')';
};
Dimensions.prototype.removeDimensions = function(howMany) {
  return _dims.slice(howMany);
};

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
  // TO DO
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
    window.open(dataURL, '_blank');
  } else if (dims.length == 4) {
    var imageCount = dims[0];
    _.each(_.range(imageCount), function(imageIndex) {
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
            var value = Math.floor(this.valueAt(imageCount, y, x, channel));
            pixelColors[channel] = value;
          }
          var color = 'rgba(' + pixelColors.join(',') + ')';
          context.fillStyle = color;
          context.fillRect(x, y, 1, 1);
        }
      }
      var dataURL = canvas.toDataURL("image/png");
      console.log(dataURL);
      window.open(dataURL, '_blank');
    });
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

Network = function(filename) {
  this._isLoaded = false;
  this._isHomebrewed = true;
  this._fileTag = null;
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
//  input.showDebugImage();
  var result = [{
    'value': 0.1,
    'label': 'sombrero',
  }];
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
  console.log(graphDict.toString());
  this._fileTag = graphDict;
  var dataMeanTag = graphDict.getTagFromDict("data_mean");
  console.assert(dataMeanTag != null);
  this._dataMean = bufferFromTagDict(dataMeanTag);
  this._dataMean.view().reshape(new Dimensions([256, 256, 3])).showDebugImage();
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
  var dataView = new DataView(arrayBuffer);
  var header = new Uint32Array(arrayBuffer, offset, 2);
  var type = header[0];
  var length = header[1];
  var valuesBuffer = arrayBuffer.slice((offset + 8), (offset + 8 + length));
  return new BinaryTag(type, length, valuesBuffer);
};

BinaryTag = function(type, length, valuesBuffer) {
  var value;
  console.log(valuesBuffer.byteLength);
  if (type === JP_CHAR) {
    var stringBytes = new Uint8Array(valuesBuffer, 0, (length-1));
    value = '';
    var index = 0;
    while (index < length) {
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