/* AG-master 13.06.25-24 (2013-06-25 11:42:54 PDT) */
rsinetsegs=['A08723_10049','A08723_10028','A08723_10052','A08723_10058','A08723_50124','A08723_0'];
    var asiExp=new Date((new Date()).getTime()+2419200000);
    var asiSegs="";
    var rsiPat=/.*_5.*/; 
    var asiPat=/\d{5}_/; 
    var i=0;
    for(x=0;x<rsinetsegs.length&&i<30;++x){if(!rsiPat.test(rsinetsegs[x])){asiSegs+='|'+rsinetsegs[x].replace(asiPat,"");++i;}}
    document.cookie="asi_segs="+(asiSegs.length>0?asiSegs.substr(1):"")+";expires="+asiExp.toGMTString()+";path=/;domain=";
    if(typeof(DM_onSegsAvailable)=="function"){DM_onSegsAvailable(['A08723_10049','A08723_10028','A08723_10052','A08723_10058','A08723_50124','A08723_0'],'a08723');}