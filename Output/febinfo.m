%+----------------------*
%|DICEA Lab Idraulica © |
%+----------------------+
%FEBINFO Open a FEB file and read the header of the file.
%febinfo(filename,verbose)
%filename:  Name of the file FEB
%verbose:   Shows the header
%           <1>: shows (default); <0>: do not shows
%by Luca Shindler 2012

function [fid,FrameNumber,StartFrame,FrameAdvance,WindowSize,ImgRows,ImgCols,XOffset,YOffset,Width,Height,MinDistance,dpMin, QualityLevel, PyrLevels, WinTrack]=febinfo(filename,verbose)
if nargin==1
   verbose=1;
end
fid=fopen(sprintf('%s.feb',filename),'rb');
FrameNumber=fread(fid,1,'uint32');
StartFrame=fread(fid,1,'int');
FrameAdvance=fread(fid,1,'int');
WindowSize=fread(fid,1,'int');
ImgRows=fread(fid,1,'int');
ImgCols=fread(fid,1,'int');
XOffset=fread(fid,1,'int');
YOffset=fread(fid,1,'int');
Width=fread(fid,1,'int');
Height=fread(fid,1,'int');
MinDistance=fread(fid,1,'float');
dpMin=fread(fid,1,'int');
QualityLevel=fread(fid,1,'float');
PyrLevels=fread(fid,1,'int');
WinTrack=fread(fid,1,'int');

%fseek(fid,512,'bof');
if verbose
   disp(sprintf('Frame number: %d\nStart Frame: %d\nFrame Advance: %d\nWindow Size: %d\nImage Rows: %d\nImage Cols: %d\nX Offset: %d\nY Offset: %d\nWidth: %d\nHeight: %d\nMinDistance: %d\nQualityLevel: %d\nPyramidal Levels: %d\nWindows Tracking: %d',FrameNumber,StartFrame,FrameAdvance,WindowSize,ImgRows,ImgCols,XOffset,YOffset,Width,Height,MinDistance,QualityLevel, PyrLevels, WinTrack));
end 

