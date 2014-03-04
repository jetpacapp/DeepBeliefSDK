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
  // TO DO
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
  // TO DO
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
          pixelColors[channel] = this.valueAt(y, x, channel);
        }
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
            pixelColors[channel] = this.valueAt(imageCount, y, x, channel);
          }
          context.fillStyle = 'rgba(' + pixelColors.join(',') + ')';
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

Network = function(filename) {
};
Network.prototype.classifyImage = function(input, doMultiSample, layerOffset) {
  input.showDebugImage();
  var result = [{
    'value': 0.1,
    'label': 'sombrero',
  }];
  return result;
};