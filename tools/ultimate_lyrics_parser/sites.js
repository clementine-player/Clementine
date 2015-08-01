/*
descriptor fields specification:
	- url - parameters: {artist},{title},{album},{Artist},{Title},{Album},{a}
	- urlFormat - specify character replacements for url parameters
	- title - use <title>{artist} - {title} lyrics</title> for extracting artist and title from returned html
	- charset
	- extract - short rule(s) for extracting reply from returned html
	- exclude - similar as extract but inverse
	- invalidIndicator - string(s) that identify returned page as invalid if page contains the invalidIndicator(s)
	- getReply - javascript function that manually extract reply from a html page
	extract, exclude and invalidIndicator fields are alternative to getReply
	extract, exclude and invalidIndicator fields can be catenated in an array

extract, exclude fields specification (2 formats):
	- string - an html tag
	- 2 lenght array - specify starting and ending string that surround data
	alternative (fallback) rules for extract are named extract2,extract3,extract4 (useful when a site use multiple templates)
*/
/*
var reply = "<FONT color=#DDDDDD><b>13. Payback</b></font>"
var start = reply.search(new RegExp("<b>\\d+. Payback<\\/b>","i"));
Amarok.alert(start);
*/
const siteDescriptors = {
	"lyrics.wikia.com": {
		title: "{artist}:{title} Lyrics - ",
		charset: "utf-8",
		extract: [["<div class='lyricbox'>","<!--"]],
//		extract2: [["<div class='lyricbox' >","<!--"]],
		exclude: ["<div class='rtMatcher'>",'<span style="padding:1em">'],
		setup: function(){
			this.multiple = settings.multi_lang_wikia;
			this.urlHashDiscriminator = this.multiple;
		},

		url: "http://lyrics.wikia.com/{Artist}:{Title}",
		urlFormat : [
			{punct: " _@;\\\"", rep: "_" },	//	removed ,/
			{punct: "?", rep: "%3F" },
		]
	},
	"lyricsplugin.com": {
		title: "{artist} - {title} Lyrics",
		charset: "utf-8",
		extract: '<div id="lyrics">',

		url: "http://www.lyricsplugin.com/winamp03/plugin/?title={title}&artist={artist}",
		urlFormat : [
			{punct: "_@;&\\/\"", rep: "-"},//	removed ,
			{punct: "'", rep: ""},
			{punct: " ", rep: "%20"},
		]
	},
	"lyricstime.com": {
		title: "{artist} - {title} Lyrics",
		charset: "iso-8859-1",
		extract: ['<div id="songlyrics" >','<p>'],

		url: "http://www.lyricstime.com/{artist}-{title}-lyrics.html",
		urlFormat : [
			{punct: " _@,;&\\/\"'", rep: "-"},
			{punct: ".", rep: ""},
		]
	},
	"lyricsreg.com": {
		title: "{title} lyrics {artist}",
		charset: "iso-8859-1",
		invalidIndicator: "Page not Found",
		extract: [['Ringtone to your Cell','Ringtone to your Cell'],['<div style="text-align:center;">','<a']],

		url: "http://www.lyricsreg.com/lyrics/{artist}/{title}/",
		urlFormat : [
			{rep: "+", punct: " _@,;&\\/\"" },
			{rep: "", punct: "'." },
		]
	},
	"lyricsmania.com": {
		title: "{artist} - {title} Lyrics",
		charset: "iso-8859-1",
		invalidIndicator: "The lyrics you requested is not in our archive yet,",
		extract: [['<span style="font-size:14px;">','<span style="font-size:14px;">'],['</center>','<a']],

		url: "http://www.lyricsmania.com/{title}_lyrics_{artist}.html",
		urlFormat : [
			{rep: "_", punct: " _@;&\\/\"'." },	//	removed ,
		]
	},
	"seeklyrics.com": {
		title: "{artist} - {title} Lyrics",
		charset: "iso-8859-1",
		extract: '<div id="songlyrics">',

		url: "http://www.seeklyrics.com/lyrics/{Artist}/{Title}.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/'\"" },
			{rep: "", punct: "." },
		]
	},
	"azlyrics.com": {
		title: "{artist} LYRICS - {title}",
		charset: "utf-8",
//		charset: "iso-8859-1",
		extract: [['<!-- END OF RINGTONE 1 -->','<!-- RINGTONE 2 -->']],
		exclude: ['<B>',['<i>[',']</i>'],['[',']']],

		url: "http://www.azlyrics.com/lyrics/{artist}/{title}.html",
		urlFormat : [
			{rep: "", punct: " ._@,;&\\/'\"-" },
		]
	},
	"metrolyrics.com": {
		title: "{artist} - {title} LYRICS",
		charset: "utf-8",
		invalidIndicator: "These lyrics are missing",
		extract: '<span id="lyrics">',
		extract2: '<div id="lyrics">',
		exclude: '<h5>',

		url: "http://www.metrolyrics.com/{title}-lyrics-{artist}.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/\"" },
			{rep: "", punct: "'." },
		]
	},
	"jamendo.com": {
		charset: "utf-8",
		getReply: function(reply){
			if(reply.indexOf('<div')>0)
				reply = reply.extractXmlTag('<div id="lyrics" style="margin-left:5px;">');
			else
				reply = reply.replace(/\n/g,"<br />")
			return reply;
		},

		url: "http://www.jamendo.com/en/get/track/list/track-artist/lyricstext/plain/?searchterm={Title}&artist_searchterm={Artist}",
//		url: "http://www.jamendo.com/en/get/track/list/track-artist-album/lyricstext/plain/?searchterm={title}&artist_searchterm={artist}",
		urlFormat : []
	},
	"mp3lyrics.org": {
		title: "{artist} &quot;{title}&quot; Lyrics",
		charset: "utf-8",
		invalidIndicator: "Something went wrong",
		extract: "<span id=gn_lyricsB>",
		extract2: '<div class="KonaBody" id="EchoTopic">',
		exclude: ["<font size=2>",["<b><i>","</u></b>:"],["<b>Lyrics","</b>"]],

		url: "http://www.mp3lyrics.org/{a}/{artist}/{title}/",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/\"" },
			{rep: "", punct: "'." },
		]
	},
	"songlyrics.com": {
		title: "{title} LYRICS - {artist}",
		charset: "utf-8",
		invalidIndicator: ["Sorry, we have no", "This is an upcoming album and we do not have the"],
		extract: '<p id="songLyricsDiv" ondragstart="return false;" onselectstart="return false;" oncontextmenu="return false;" class="songLyricsV14" style="font-size: 14px;z-index: 9999;position: absolute;left: -6000px;">',
		exclude: [['\n[','] ']],

		url: "http://www.songlyrics.com/{artist}/{title}-lyrics/",
		urlFormat : [
			{rep: "-", punct: " ._@,;&\\/\"" },
			{rep: "_", punct: "'" },
		]
	},
	"darklyrics.com": {
		title: "{artist} LYRICS - {title}",
		charset: "utf-8",
		getReply: function(reply, info){
//			var start = reply.search(new RegExp(("<FONT color=#DDDDDD><b>\d+. " + info.arrange(info.title,this.urlFormat,false) + "</b></font>").escapeRegExp(),"i"));
			var start = reply.search(new RegExp("<b>\\d+. " + info.arrange(info.title,SearchInfo.prototype.defaultReplaces,false) + "[^<]*<\\/b>","i"));
//			var start = reply.search(new RegExp("<FONT color=#DDDDDD>","i"));
			if (start<0)
				return "";
//			Amarok.alert(start);
			reply = reply.substring(start);
			var end = reply.search(new RegExp("<font","i"));
			reply = reply.substring(0,end);
			reply = reply.substring(reply.indexOf("</b>")+4);
			reply = reply.replace(/<\/font>/ig,"");
			reply = reply.replace(/<a name=\d+>/ig,"");

			return reply;
		},
/*
		matchInfo:function(reply,info,info2){
			if (info2){
				info.matchedTitle = info2.title;
				info.matchedArtist = info2.artist;
//			Amarok.alert(info.matchedTitle + " | " + info.matchedArtist);
			}else{
				LyricsClass.prototype.matchInfo.call(this,reply,info);
				if (!info.arrange)
					return;
				var title = reply.match(new RegExp("<b>\\d+. (" + info.arrange(info.title,SearchInfo.prototype.defaultReplaces,false) + "[^<]*)<\\/b>","i"));
				if (title)
					info.matchedTitle = title[1];
			}
			Amarok.alert(info.matchedTitle + " | " + info.matchedArtist);
		},
*/
		invalidIndicator: "Not Found",

		url: "http://www.darklyrics.com/lyrics/{artist}/{album}.html",
		urlFormat : [
			{rep: "", punct: " _@,;&\\/\"'." },
		]
	},
	"lyricsmode.com": {
		title: "{artist} - {title} lyrics",
		charset: "iso-8859-1",
		invalidIndicator: "Sorry, we have no",
//		extract: '<div id="songlyrics">',
//		extract2: '<span id="lyrics">',
		extract: "<div id='songlyrics_h' class='dn'>",

		url: "http://www.lyricsmode.com/lyrics/{a}/{artist}/{title}.html",
		urlFormat : [
			{rep: "_", punct: " ._@,;&\\/\"" },
		]
	},
	"elyrics.net": {
		title: "{title} Lyrics - {artist}",
		charset: "iso-8859-1",
		invalidIndicator: "Page not Found",
		extract : "<div class='ly' style='font-size:12px;'>",
		exclude : ["<strong>","<em>"],

		url: "http://www.elyrics.net/read/{a}/{artist}-lyrics/{title}-lyrics.html",
		urlFormat : [
			{rep: "-", punct: " _@;&\\/\"" },	//	removed ,
			{rep: "_", punct: "'" },
		]
	},
	"lyricsdownload.com": {
		title: "{artist} - {title} LYRICS",
		charset: "utf-8",
		invalidIndicator: "We haven't lyrics of this song",
		extract : '<div id="div_customCSS">',

		url: "http://www.lyricsdownload.com/{artist}-{title}-lyrics.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/\"" },
			{rep: "", punct: "." },
		]
	},
	"lyrics.com": {
		title: "{artist} - {title} Lyrics",
		charset: "utf-8",
		invalidIndicator: "we do not have the lyric for this song",
		extract: '<div id="lyrics" class="SCREENONLY">',
//		extract: '<div id="lyric_space">',

		url: "http://www.lyrics.com/lyrics/{artist}/{title}.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/\"" },
			{rep: "", punct: "'." },
		]
	},
	"lyriki.com": {
		charset: "utf-8",
		extract: [['</table>','<div class="printfooter">'],'<p>'],

		url: "http://www.lyriki.com/{artist}:{title}",
		urlFormat : [
			{rep: "_", punct: " _@,;&\\/\"" },
			{rep: "", punct: "." },
		]
	},
	"lyricsbay.com": {
		title: "{title} lyrics {artist}",
		charset: "iso-8859-1",
		extract: "<div id=EchoTopic>",
		exclude: '<textarea name="songscpy" id="songscpyid" onclick="callselect(\'songscpyid\',\'selectswf\')" rows="3" cols="45" READONLY>',
//		exclude: [["<i>[","]</i>"]],

		url: "http://www.lyricsbay.com/{title}_lyrics-{artist}.html",
		urlFormat : [
			{rep: "_", punct: " _@,;&\\/'\"" },
			{rep: "", punct: "." },
		]
	},
	"directlyrics.com": {
		title: "{artist} - {title} lyrics",
		charset: "iso-8859-1",
		extract: ['<div id="lyricsContent">','<p>'],
		exclude: [["<b>","</b>"]],

		url: "http://www.directlyrics.com/{artist}-{title}-lyrics.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/'\"" },
			{rep: "", punct: "." },
		]
	},
	"loudson.gs": {
		charset: "utf-8",
		extract : '<div class="middle_col_TracksLyrics ">',

		url: "http://www.loudson.gs/{a}/{artist}/{album}/{title}",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/\"" },
			{rep: "", punct: "." },
		]
	},
	"lyricsfreak.com": {
//		title: "{artist} | {title} lyrics",
		title: "{artist}, {title}",
		charset: "iso-8859-1",
		extract: "<div id='content_h' class='dn'>",
//		extract : '<div id="content" style="float: none; clear: both;FONT-SIZE: 15px; FONT-FAMILY: \'Times New Roman\', serif;" lang="en">'
	},
	"sing365.com": {
		title: "{artist} - {title} LYRICS",
		charset: "utf-8",
		extract: [['<img src=http://www.sing365.com/images/phone2.gif border=0><br><br></div>','<div']]
	},
	"allreggaelyrics.com": {
		charset: "iso-8859-1",
		getReply: function(reply){
			reply = reply.extractXmlTag("<pre>");
			reply = reply.replace(/\n/g,'<br/>\n');
			return reply;
		}
	},
	"stixoi.info (Greek songs)": {
		charset: "utf-8",
		extract: [['<font class=creators>','</td>']]
	},
	"teksty.org": {
		title: "{artist} - {title} - tekst",
		charset: "utf-8",
		extract: [['<div class="songText" id="songContent">','</div>']],

		url: "http://teksty.org/{artist},{title},tekst-piosenki",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/\"'" },
			{rep: "", punct: "."},
		]
	},
	"tekstowo.pl (Polish translations)": {
		title: "{artist} - {title} - tekst",
		charset: "utf-8",
		extract: [['<div class="song-text">','<a href="javascript:;"']],	//	original
		extract2: '<div class="tlumaczenie">',	//	translated
		exclude: [["<h2>","</h2><br />"]],

		url: "http://www.tekstowo.pl/piosenka,{artist},{title}.html",
		urlFormat : [
			{rep: "_", punct: " _@,;&\\/'\"." },
		]
	},
	"vagalume.com.br": {
		title: "{title} de {artist} no VAGALUME",
		charset: "iso-8859-1",
		extract: '<span class="editable_area">',
//		exclude: [['<div style="float:left"','>']],

		url: "http://vagalume.com.br/{artist}/{title}.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/'\"." },
		]
	},
	"vagalume.com.br (Portuguese translations)": {
		title: "{title} de {artist} no VAGALUME",
		charset: "iso-8859-1",
		extract: '<div class="tab_traducao sideBySide lyricArea tab_tra_pt">',
//		exclude: [['<div style="float:left"','>']],

		url: "http://vagalume.com.br/{artist}/{title}-traducao.html",
		urlFormat : [
			{rep: "-", punct: " _@,;&\\/'\"." },
		]
	},
	"zeneszoveg.hu": {
		title: "{artist} : {title} - dalszöveg",
		charset: "utf-8",
		extract: [['</a>?','<div']]
	},
	"letras.mus.br": {
		charset: "utf-8",
		url: "http://letras.terra.com.br/winamp.php?musica={title}&artista={artist}",
		urlFormat : [
			{rep: "_", punct: "_@,;&\\/\"" },
			{rep: "+", punct: " " },
		],
		extract: [["</div>","</div>"]],
		invalidIndicator: "Verifique se o nome do seu arquivo e sua",
	},
	"lololyrics.com": {
		charset: "utf-8",
		url: "http://api.lololyrics.com/0.5/getLyric?artist={artist}&track={title}",
		urlFormat : [
			{rep: "_", punct: "_@,;&\\/\"#" },
		],
		extract: '<response>',
		invalidIndicator: "ERROR",
	},
}

//	--------------------------------------------------------------------------------------------------------------------------------

function UniversalSite(){

	this.charset = "utf-8";
	this.urlHashDiscriminator = settings.max_size_html + settings.max_size_lyrics;
	this.postExtractReplaces = [];

 	this.getReply = function(reply){
		if (!arguments.callee.preReplaces){
			arguments.callee.preReplaces = [
				{expr:/[\r\t\n]/g, str:""},				//	removes tab and return and new line chars
				{expr:/ +/g, str:" "},					//	compress multiple spaces
				{expr:/<script[^>]*(?!<\/script>)*<\/script>/mig , str:""},	//	removes <script> tag
				{expr:/<!--(?!-->)*-->/mig , str:""},			//	removes html comments <!-- -->
				{expr:/<[^>]*[biup]>/ig , str:""},			//	removes xml tags <b> <i> <p>...
			];
			arguments.callee.postReplaces = [
				{expr:/\n((<[^>\n]*>)| )*\n/g, str:""},			//	removes empty lines
				{expr:/\n+/g, str:"\n"},				//	removes empty lines
			];
		}
//		reply = reply.excludeAll("<script","/script>");
		reply = reply.multiReplace(arguments.callee.preReplaces);
		
		reply = reply.match(/>([^<]*)<\/?[biup]/mig); 				// anything that has <br>
		if (reply == null)
			reply = [];
		var html = "";
		for (i=0;i<reply.length;i++)			
			html+=reply[i].match(/>([^<]*)<\/?[biup]/mi)[1] + "<br/>";

		html = html.multiReplace(arguments.callee.postReplaces);
		html = html.multiReplace(LyricsClass.prototype.postExtractReplaces);

		if (html == "")
			html = "<i>{msg:no text found}</i>";
		return html;
	}

	this.downloadableReply = function(info){
		return info.url!="";
	}

}

//	--------------------------------------------------------------------------------------------------------------------------------

String.prototype.encodeSuggestionURL = function(){
	return this.toString();
}

String.prototype.decodeSuggestionURL = function(){
	return this.toString();
}

String.prototype.formatSuggestionInfo = function(){
	return this.toString();
}

function GLyrics(){
	this.charset = "utf-8";
	this.display_mode = "Suggestions";
	this.url = "http://www.google.com/search?num={gsearch_no}&q={artist}+{title}+{gsearch}";
	this.urlFormat = [
		{punct: "&", rep: "" }
	];
	this.urlHashDiscriminator = settings.gsearch_filter_expr + settings.gsearch_filter_url + binToInt(settings.gsearch_filter_artist);
//	alert(this.urlHashDiscriminator);

	this.getUrl = function(info){
		var url = LyricsClass.prototype.getUrl.call(this,info);
		url = url.fill1(settings);
//		url = url.replace(/\ /g,"+");
		return url;
	}

 	this.getReply = function(reply,info){
		if (!arguments.callee.filterURL){
			arguments.callee.filterURL = function(url){
				if(!arguments.callee.filter_expr){
					var filter_expr = settings.gsearch_filter_expr.split("|");
					for(var i in filter_expr)
						filter_expr[i] = "(" + filter_expr[i].escapeRegExp() + ")";
					arguments.callee.filter_expr = new RegExp(filter_expr.join("|"),"i");
				}

				if (settings.gsearch_filter_url == "no filter")
					return true;
				if (settings.gsearch_filter_url == "hostname")
					url = (new QUrl(url)).host();
				return url.search(arguments.callee.filter_expr)>=0;
			}
			arguments.callee.filterTitle = function(title, info){
				var b = new RegExp(info.artist,"i").test(title);
//				if (!b)	alert(title);
				return b;
			}
		}
		if (!info.isValid)
			return "not found";

		reply = reply.replace(/<\/?em>/g,"");
		reply = reply.replace(/<\/?b>/g,"");
		var suggestions = reply.match( /href=\"([^\"]*)\" class=l[^>]*>([^<]*)<\/a>/g );
		if (suggestions === null || !suggestions)
			return "not found";
//		Amarok.alert(suggestions);
		var fullySupported = "";
		var universalSupported = "";
		for (i=0;i<suggestions.length;i++){
			suggestions[i] = suggestions[i].match(/href=\"([^\"]*)\" class=l[^>]*>([^<]*)<\/a>/);
			var url = suggestions[i][1].escapeXmlChars().encodeSuggestionURL();
			var title = suggestions[i][2];
			if (!arguments.callee.filterURL(url) || (settings.gsearch_filter_artist && !arguments.callee.filterTitle(title, info)))
				continue;
			var site = SiteManager.domains[(new QUrl(url)).host()];
			var inf = {artist:info.artist,title:title};
			var bFullySupported = !(!site);
			if (!site)
				site = this;

			site.matchInfo("<title>"+inf.title+"</title>",inf,info);
//			alert(site.domain + " " + SiteManager.domains[(new QUrl(url)).host()] + " " + (new QUrl(url)).host());
			var artist = inf.matchedArtist.escapeNumberedChars().escapeHtmlEntities().capitalize();
			var title = inf.matchedTitle.escapeNumberedChars().escapeHtmlEntities().capitalize();
			var host = " (" + (new QUrl(url)).host()+")";
			if (artist == ""){
				artist = "unknown";
//				bFullySupported = false;
//				artist = "(" + (new QUrl(url)).host()+")";
//			if (title.length > 40 - artist.length - host.length)
//				title = title.substr(0, 40 - artist.length - host.length) + "..";
			}
			title += host;
//			Amarok.alert(url);
			var suggestion = "<suggestion artist=\"" + artist.formatSuggestionInfo().replace(/\"/g,"'").escapeXmlChars() + "\" title=\"" + title.formatSuggestionInfo().replace(/\"/g,"'").escapeXmlChars()+ "\" url=\"" + url + "\" />\n";
			if (bFullySupported)
				fullySupported += suggestion;
			else
				universalSupported += suggestion;
		}
		var xml = fullySupported+universalSupported;
		if (xml=="")
			return "not found";
		return xml;
	}

	this.filter = function(info){
		return true;
	}

	this.downloadableReply = function(info){
		return info.url=="" && !this.skipSearch;
	}
}

//	--------------------------------------------------------------------------------------------------------------------------------

function NotFound(){
	this.display_mode = "Suggestions";

 	this.getReply = function(reply,info){
		return "not found";
	}
/*
	this.downloadableReply = function(info){
		return false;
	};
*/
	this.filter = function(info){
		return !info.isValid;
	}

}

