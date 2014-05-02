var MINIMUM_FONT = "10";
var UNITS = "";

function elementFontSize(element)
{
    var fontSize = MINIMUM_FONT; 

    if (document.defaultView)
    {
        var computedStyle = document.defaultView.getComputedStyle(element, null);
        if (computedStyle)
        {
            fontSize = computedStyle.getPropertyValue("font-size");
        }
    }
    else if (element.currentStyle)
    {
        fontSize = element.currentStyle.fontSize;
    }

    if ((UNITS.length == 0) && (fontSize != MINIMUM_FONT))
    {
        UNITS = fontSize.substring(fontSize.length - 2, fontSize.length)
    }

    return parseFloat(fontSize);
}

function adjustFontSizeIfTooBig(idOfElement)
{
    var oTextBoxOuterDiv;
    var oTextBoxMiddleDiv;
    var oTextBoxInnerDiv;
    var oTextBoxOuterDiv = document.getElementById(idOfElement);
    
    if (oTextBoxOuterDiv)
    {
        oTextBoxMiddleDiv = getChildOfType(oTextBoxOuterDiv, "DIV", 0);
        if (oTextBoxMiddleDiv)
        {
            oTextBoxInnerDiv = getChildOfType(oTextBoxMiddleDiv, "DIV", 0);
            if (oTextBoxInnerDiv)
            {
                var offsetHeight = oTextBoxInnerDiv.offsetHeight;
                var specifiedHeight = offsetHeight;
                if (oTextBoxMiddleDiv.style.height != "")
                {
                    specifiedHeight = parseFloat(oTextBoxMiddleDiv.style.height);
                }
                else if (oTextBoxOuterDiv.style.height != "")
                {
                    specifiedHeight = parseFloat(oTextBoxOuterDiv.style.height);
                }
                if (offsetHeight > specifiedHeight)
                {
                    var smallestFontSize = 200;
                    
                    var aParaChildren = getParaDescendants(oTextBoxInnerDiv);
                    var oneLine = false;
                    for (i = 0; i < aParaChildren.length; i++)
                    {
                        var oParagraphDiv = aParaChildren[i];
                        var lineHeight = elementLineHeight(oParagraphDiv);
                        oneLine = oneLine || (lineHeight * 1.5 >= specifiedHeight);
                        if (oParagraphDiv.nodeName == "DIV")
                        {
                            var fontSize = elementFontSize(oParagraphDiv);
                            smallestFontSize = Math.min( smallestFontSize, fontSize );
                            for (j = 0; j < oParagraphDiv.childNodes.length; j++)
                            {
                                var oSpan = oParagraphDiv.childNodes[j];
                                if ((oSpan.nodeName == "SPAN") || (oSpan.nodeName == "A"))
                                {
                                    fontSize = elementFontSize(oSpan);
                                    smallestFontSize = Math.min( smallestFontSize, fontSize );
                                }
                            }
                        }
                    }
                    var minimum = parseFloat(MINIMUM_FONT);
                    
                    var count = 0
                    while ((smallestFontSize > minimum) && (offsetHeight > specifiedHeight) && (count < 10))
                    {
                        ++ count;
                        if (oneLine)
                        {
                            var oldWidth = parseInt(oTextBoxOuterDiv.style.width);
                            oTextBoxInnerDiv.style.width =
                                "" + oldWidth * Math.pow(1.05, count) + "px";
                        }
                        else
                        {
                            var scale = Math.max(0.95, minimum / smallestFontSize);
                            
                            for (i = 0; i < aParaChildren.length; i++)
                            {
                                var oParagraphDiv = aParaChildren[i];
                                if (oParagraphDiv.nodeName == "DIV")
                                {
                                    var paraFontSize = elementFontSize(oParagraphDiv) * scale;
                                    var paraLineHeight = elementLineHeight(oParagraphDiv) * scale;
                                    for (j = 0; j < oParagraphDiv.childNodes.length; j++)
                                    {
                                        var oSpan = oParagraphDiv.childNodes[j];
                                        if ((oSpan.nodeName == "SPAN") || (oSpan.nodeName == "A"))
                                        {
                                            var spanFontSize = elementFontSize(oSpan) * scale;
                                            var spanLineHeight = elementLineHeight(oSpan) * scale;
                                            oSpan.style.fontSize = spanFontSize + UNITS;
                                            oSpan.style.lineHeight = spanLineHeight + UNITS;
                                            smallestFontSize = Math.min( smallestFontSize, spanFontSize );
                                        }
                                    }
                                    oParagraphDiv.style.fontSize = paraFontSize + UNITS;
                                    oParagraphDiv.style.lineHeight = paraLineHeight + UNITS;
                                    smallestFontSize = Math.min( smallestFontSize, paraFontSize );
                                }
                            }
                        }
                        
                        offsetHeight = oTextBoxInnerDiv.offsetHeight;
                    }
                }
            }
        }
    }
}


function elementLineHeight(element)
{
    var lineHeight = MINIMUM_FONT; 
    
    if (document.defaultView)
    {
        var computedStyle = document.defaultView.getComputedStyle(element, null);
        if (computedStyle)
        {
            lineHeight = computedStyle.getPropertyValue("line-height");
        }
    }
    else if (element.currentStyle)
    {
        lineHeight = element.currentStyle.lineHeight;
    }
    
    if ((UNITS.length == 0) && (lineHeight != MINIMUM_FONT))
    {
        UNITS = lineHeight.substring(lineHeight.length - 2, lineHeight.length)
    }
    
    return parseFloat(lineHeight);
}

function adjustLineHeightIfTooBig(idOfElement)
{
    var oTextBoxOuterDiv;
    var oTextBoxMiddleDiv;
    var oTextBoxInnerDiv;
    var oTextBoxOuterDiv = document.getElementById(idOfElement);
    
    if (oTextBoxOuterDiv)
    {
        oTextBoxMiddleDiv = getChildOfType(oTextBoxOuterDiv, "DIV", 0);
        if (oTextBoxMiddleDiv)
        {
            oTextBoxInnerDiv = getChildOfType(oTextBoxMiddleDiv, "DIV", 0);
            if (oTextBoxInnerDiv)
            {
                var offsetHeight = oTextBoxInnerDiv.offsetHeight;
                var specifiedHeight = offsetHeight;
                if (oTextBoxMiddleDiv.style.height != "")
                {
                    specifiedHeight = parseFloat(oTextBoxMiddleDiv.style.height);
                }
                else if (oTextBoxOuterDiv.style.height != "")
                {
                    specifiedHeight = parseFloat(oTextBoxOuterDiv.style.height);
                }
                if (offsetHeight > specifiedHeight)
                {
                    var adjusted = true;
                    var count = 0;
                    while ((adjusted) && (offsetHeight > specifiedHeight) && (count < 10))
                    {
                        adjusted = false;
                        ++ count;
                        
                        var aParaChildren = getParaDescendants(oTextBoxInnerDiv);
                        for (i = 0; i < aParaChildren.length; i++)
                        {
                            var oParagraphDiv = aParaChildren[i];
                            if (oParagraphDiv.nodeName == "DIV")
                            {
                                var fontSize = elementFontSize(oParagraphDiv);
                                var lineHeight = elementLineHeight(oParagraphDiv) * 0.95;
                                if (lineHeight >= (fontSize * 1.1))
                                {
                                    oParagraphDiv.style.lineHeight = lineHeight + UNITS;
                                    adjusted = true;
                                }
                                
                                
                                
                                for (j = 0; j < oParagraphDiv.childNodes.length; j++)
                                {
                                    var oSpan = oParagraphDiv.childNodes[j];
                                    if ((oSpan.nodeName == "SPAN") || (oSpan.nodeName == "A"))
                                    {
                                        var fontSize = elementFontSize(oSpan);
                                        var lineHeight = elementLineHeight(oSpan) * 0.95;
                                        if (lineHeight >= (fontSize * 1.1))
                                        {
                                            oSpan.style.lineHeight = lineHeight + UNITS;
                                            var adjusted = true;
                                        }
                                    }
                                }
                            }
                        }
                        
                        offsetHeight = oTextBoxInnerDiv.offsetHeight;
                    }
                }
            }
        }
    }
}

var smallTransparentGif = "";
function fixupIEPNG(strImageID, transparentGif) 
{
    smallTransparentGif = transparentGif;
    if (windowsInternetExplorer && (browserVersion < 7))
    {
        var img = document.getElementById(strImageID);
        if (img)
        {
            var src = img.src;
            img.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + src + "', sizingMethod='scale')";
            img.src = transparentGif;
            img.attachEvent("onpropertychange", imgPropertyChanged);
        }
    }
}

function fixupIEPNGBG(oBlock) 
{
    if (oBlock)
    {
        var currentBGImage = oBlock.currentStyle.backgroundImage;
        var currentBGRepeat = oBlock.currentStyle.backgroundRepeat;
        var urlStart = currentBGImage.indexOf('url(');
        var urlEnd = currentBGImage.indexOf(')', urlStart);
        var imageURL = currentBGImage.substring(urlStart + 4, urlEnd);

        if (imageURL.charAt(0) == '"')
        {
            imageURL = imageURL.substring(1);
        }
        
        if (imageURL.charAt(imageURL.length - 1) == '"')
        {
            imageURL = imageURL.substring(0, imageURL.length - 1);
        }

        var overrideRepeat = false;

        var filterStyle =
            "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" +
            imageURL +
            "', sizingMethod='crop');";

        if (RegExp("/C[0-9A-F]{8}.png$").exec(imageURL) != null)
        {
            filterStyle =
                "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" +
                imageURL +
                "', sizingMethod='scale');";

            overrideRepeat = true;
        }

        var backgroundImage = new Image();
        backgroundImage.src = imageURL;
        var tileWidth = backgroundImage.width;
        var tileHeight = backgroundImage.height; 
        
        var blockWidth = 0;
        var blockHeight = 0;
        if (oBlock.style.width)
        {
            blockWidth = parseInt(oBlock.style.width);
        }
        else
        {
            blockWidth = oBlock.offsetWidth;
        }
        if (oBlock.style.height)
        {
            blockHeight = parseInt(oBlock.style.height);
        }
        else
        {
            blockHeight = oBlock.offsetHeight;
        }

        if ((blockWidth == 0) || (blockHeight == 0))
        {
            return;
        }
        
        var wholeRows = 1;
        var wholeCols = 1;
        var extraHeight = 0;
        var extraWidth = 0;
        
        if ((currentBGRepeat.indexOf("no-repeat") != -1) ||
              ((tileWidth == 0) && (tileHeight == 0)) ||
              overrideRepeat)
        {
            tileWidth = blockWidth;
            tileHeight = blockHeight;

        }
        else if ((currentBGRepeat.indexOf("repeat-x") != -1) ||
              (tileHeight == 0))
        {
            wholeCols = Math.floor(blockWidth / tileWidth);
            extraWidth = blockWidth - (tileWidth * wholeCols);
            tileHeight = blockHeight;

        }
        else if (currentBGRepeat.indexOf("repeat-y") != -1)
        {
            wholeRows = Math.floor(blockHeight / tileHeight);
            extraHeight = blockHeight - (tileHeight * wholeRows);
            tileWidth = blockWidth;

        }
        else
        {
            wholeCols = Math.floor(blockWidth / tileWidth);
            wholeRows = Math.floor(blockHeight / tileHeight);
            extraWidth = blockWidth - (tileWidth * wholeCols);
            extraHeight = blockHeight - (tileHeight * wholeRows);
        }
        
        var wrappedContent = document.createElement("div");
        wrappedContent.style.position = "relative";
        wrappedContent.style.zIndex = "1";
        wrappedContent.style.left = "0px";
        wrappedContent.style.top = "0px";
        if (!isNaN(parseInt(oBlock.style.width)))
        {
            wrappedContent.style.width = "" + blockWidth + "px";
        }
        if (!isNaN(parseInt(oBlock.style.height)))
        {
            wrappedContent.style.height = "" + blockHeight + "px";
        }
        var pngBGFixIsWrappedContentEmpty = true;
        while (oBlock.hasChildNodes())
        {
            if (oBlock.firstChild.nodeType == 3)
            {
                if (RegExp("^ *$").exec(oBlock.firstChild.data) == null)
                {
                    pngBGFixIsWrappedContentEmpty = false;
                }
            }
            else
            {
                pngBGFixIsWrappedContentEmpty = false;
            }
            wrappedContent.appendChild(oBlock.firstChild);
        }
        if (pngBGFixIsWrappedContentEmpty)
        {
            wrappedContent.style.lineHeight = "0px";
        }
        
        var newMarkup = "";
        for (var currentRow = 0; 
             currentRow < wholeRows; 
             currentRow++)
        {
            for (currentCol = 0; 
                 currentCol < wholeCols; 
                 currentCol++)
            {
                newMarkup += "<div style=" +
                        "\"position: absolute; line-height: 0px; " +
                        "width: " + tileWidth + "px; " +
                        "height: " + tileHeight + "px; " +
                        "left:" + currentCol *  tileWidth + "px; " +
                        "top:" + currentRow *  tileHeight + "px; " +
                        "filter:" + filterStyle + 
                        "\" > </div>";
            }
            
            if (extraWidth != 0)
            {
                newMarkup += "<div style=" +
                        "\"position: absolute; line-height: 0px; " +
                        "width: " + extraWidth + "px; " +
                        "height: " + tileHeight + "px; " +
                        "left:" + currentCol *  tileWidth + "px; " +
                        "top:" + currentRow *  tileHeight + "px; " +
                        "filter:" + filterStyle + 
                        "\" > </div>";
            }
        }
        
        if (extraHeight != 0)
        {
            for (currentCol = 0; 
                 currentCol < wholeCols; 
                 currentCol++)
            {
                newMarkup += "<div style=" +
                        "\"position: absolute; line-height: 0px; " +
                        "width: " + tileWidth + "px; " +
                        "height: " + extraHeight + "px; " +
                        "left:" + currentCol *  tileWidth + "px; " +
                        "top:" + currentRow *  tileHeight + "px; " +
                        "filter:" + filterStyle + 
                        "\" > </div>";
            }
            
            if (extraWidth != 0)
            {
                newMarkup += "<div style=" +
                        "\"position: absolute; line-height: 0px; " +
                        "width: " + extraWidth + "px; " +
                        "height: " + extraHeight + "px; " +
                        "left:" + currentCol *  tileWidth + "px; " +
                        "top:" + currentRow *  tileHeight + "px; " +
                        "filter:" + filterStyle + 
                        "\" > </div>";
            }
        }
        oBlock.innerHTML = newMarkup;

        oBlock.appendChild(wrappedContent);
        oBlock.style.background= "";
    }
}

function fixupAllIEPNGBGs()
{
    if (windowsInternetExplorer && (browserVersion < 7))
    {
        try
        {
            var oDivNodes = document.getElementsByTagName('DIV');
            for (var iIndex=0; iIndex<oDivNodes.length; iIndex++)
            {
                var oNode = oDivNodes.item(iIndex);
                if (oNode.currentStyle &&
                    oNode.currentStyle.backgroundImage &&
                    (oNode.currentStyle.backgroundImage.indexOf('url(') != -1) &&
                    (oNode.currentStyle.backgroundImage.indexOf('.png")') != -1))
                {
                    fixupIEPNGBG(oNode);
                }
            }
        }
        catch (e)
        {
        }
    }
}

function getChildOfType(oParent, sNodeName, requestedIndex)
{
    var childrenOfType = oParent.getElementsByTagName(sNodeName);
    return (requestedIndex < childrenOfType.length) ?
           childrenOfType.item(requestedIndex) : null;
}

function getParaDescendants(oAncestor)
{
    var oParaDescendants = new Array();
    var oPotentialParagraphs = oAncestor.getElementsByTagName('DIV');
    for (var iIndex=0; iIndex<oPotentialParagraphs.length; iIndex++)
    {
        var oNode = oPotentialParagraphs.item(iIndex);
        if (oNode.className.lastIndexOf('paragraph') != -1)
        {
            oParaDescendants.push(oNode);
        }
    }
    return oParaDescendants;
}

function onPageLoad()
{
    detectBrowser();
    adjustLineHeightIfTooBig("id1");
    adjustFontSizeIfTooBig("id1");
    adjustLineHeightIfTooBig("id2");
    adjustFontSizeIfTooBig("id2");
    adjustLineHeightIfTooBig("id4");
    adjustFontSizeIfTooBig("id4");
    fixupAllIEPNGBGs();
    fixupIEPNG("id3", "MyRecorder Help_files/transparent.gif");
    return true;
}

function NBmouseover(index)
{
    var normal = document.getElementById("navbar_"+index+"_normal");
    var rollover = document.getElementById("navbar_"+index+"_rollover");
    if (normal && rollover)
    {
        normal.style.visibility = "hidden";
        rollover.style.visibility = "visible";
    }
    return true;
}

function NBmouseout(index)
{
    var normal = document.getElementById("navbar_"+index+"_normal");
    var rollover = document.getElementById("navbar_"+index+"_rollover");
    if (normal && rollover)
    {
        normal.style.visibility = "visible";
        rollover.style.visibility = "hidden";
    }
    return true;
}

var windowsInternetExplorer = false;
var browserVersion = 0;
function detectBrowser()
{
    windowsInternetExplorer = false;
    var appVersion = navigator.appVersion;
    if ((appVersion.indexOf("MSIE") != -1) &&
        (appVersion.indexOf("Macintosh") == -1))
    {
        var temp = appVersion.split("MSIE");
        browserVersion = parseFloat(temp[1]);
        windowsInternetExplorer = true;
    }
}

var inImgPropertyChanged = false;
function imgPropertyChanged()
{
    if ((window.event.propertyName == "src") && (! inImgPropertyChanged))
    {
        inImgPropertyChanged = true;
        var el = window.event.srcElement;
        if (el.src != smallTransparentGif)
        {
            el.filters.item(0).src = el.src;
            el.src = smallTransparentGif;
        }
        inImgPropertyChanged = false;
    }
}

