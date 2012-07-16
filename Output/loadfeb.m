%+---------------------+
%|DICEA Lab Idraulica  |
%+---------------------+
%loadFEB loads the barycentres of particles from a 'feb' file and the
%displacement vector predicted from a 'ofp' file.
%
%[VS,Y,np,pausa]=loadfeb(filename,pause)
%filename: Name of 'feb' file
%VS: displacement vector computed with optical flow;
%Y: coordinates of particle centres detected;
%np: number of particles detected per each frame.
%
%by Luca Shindler 2012

function [VS,Y,np,pause]=loadfeb(filename,pause)
Y=[];
YS=[];
VS=[];
if nargin==1   
   pause=0;   
end
[fid,FrameNumber,StartFrame,FrameAdvance,WindowSize,ImgRows,ImgCols,XOffset,YOffset,Width,Height,MinDistance,dpMin,QualityLevel,PyrLevels, WinTrack]=febinfo(filename);
FN=input('Type the number of frames to analyse');
fidO=fopen(sprintf('%s.ofp',filename),'rb');
np=zeros(FrameNumber+1,1);
for frm=1:FN
    frm
   nsp=fread(fid,1,'short');
   np(frm)=nsp;
   if (nsp>0)
      X=fread(fid,[2,nsp],'float');
      S=fread(fidO,[2,nsp],'float'); 
      X(2,:)=ImgRows-X(2,:);
      S(2,:)=ImgRows-S(2,:);
      V=S-X;
      Y=[Y X];
      YS=[YS S];
      VS=[VS V]; 
   end
   if(pause)
      clf;axis([0 ImgCols-1 0 ImgRows-1]);hold on;  
      quiver(X(1,:),X(2,:),V(1,:),V(2,:),3,'r');
      title(sprintf('Frame %d',frm));
      drawnow;
   end
end
fclose(fid);
fclose(fidO);