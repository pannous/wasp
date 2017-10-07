(function(f){if(typeof exports==="object"&&typeof module!=="undefined"){module.exports=f()}else if(typeof define==="function"&&define.amd){define([],f)}else{var g;if(typeof window!=="undefined"){g=window}else if(typeof global!=="undefined"){g=global}else if(typeof self!=="undefined"){g=self}else{g=this}g.MarkConvert=f()}})(function(){var define,module,exports;return function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s}({"/lib/mark.convert.js":[function(require,module,exports){const $length=Symbol.for("Mark.length");const $parent=Symbol.for("Mark.parent");const $pragma=Symbol.for("Mark.pragma");var MarkConvert=function(Mark){let m=Mark;function toMark(elmt){if(!elmt)return null;var obj=m(elmt.tagName.toLowerCase());if(elmt.hasAttributes()){var attrs=elmt.attributes;for(var i=0;i<attrs.length;i++){var attr=attrs[i];if(attr.specified)obj[attr.name]=attr.value}}if(elmt.hasChildNodes()){for(var i=0;i<elmt.childNodes.length;i++){var child=elmt.childNodes[i];if(child.nodeType===3){obj.push(child.textContent)}else if(child.nodeType===1){obj.push(toMark(child))}}}return obj}MarkConvert.parse=function(source,options){if(typeof document!=="undefined"){var doc=document.implementation.createHTMLDocument();if(source.match(/^\s*(<!doctype|<html)/i)){console.log("parse whole doc");doc.documentElement.innerHTML=source;console.log("doc elmt",doc.documentElement);return toMark(doc.documentElement)}else{console.log("parse doc fragment");doc.body.innerHTML=source;var children=doc.body.children;if(children&&children.length>1){var result=[];for(var i=0;i<children.length;i++){result.push(toMark(children[i]))}return result}else{return toMark(doc.body.children)}}}else{var htmlparser=require("htmlparser2");var root=null,parent=null,stack=[];var opt={decodeEntities:true};if(options){if(options.format=="xml"){opt.xmlMode=true}}var parser=new htmlparser.Parser({onopentag:function(name,attribs){var obj=m(name,attribs,null,parent);if(parent){parent.push(obj);stack.push(parent);parent=obj}else{parent=obj;if(!root)root=obj}},ontext:function(text){if(/^\s*$/.test(text)){}else{parent.push(text)}},oncomment:function(comment){var comt={};comt[$parent]=parent;comt[$pragma]="!--"+comment;parent.push(comt)},onclosetag:function(tagname){parent=stack.pop()}},opt);parser.write(source.trim());parser.end();return root}};var htmlEscapes={"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#x27;"};var htmlEscaper=/[&<>"']/g;function escapeStr(str){return(""+str).replace(htmlEscaper,function(match){return htmlEscapes[match]})}MarkConvert.toHtml=function(object){var emptyTags={area:1,base:1,basefont:1,br:1,col:1,frame:1,hr:1,img:1,input:1,isindex:1,link:1,meta:1,param:1,embed:1};function htm(obj){if(typeof obj!=="object"){return""}if(obj[$pragma]){return(obj[$pragma].startsWith("!--")?"<":"\x3c!--")+obj[$pragma]+"--\x3e"}var buffer="<"+obj.constructor.name;for(var prop in obj){var value=obj[prop];if(value!=null){buffer+=" "+prop+'="'+escapeStr(value)+'"'}}buffer+=">";var length=obj[$length];for(var i=0;i<length;i++){var item=obj[i];if(typeof item==="string"){buffer+=escapeStr(item)}else if(typeof item==="object"){buffer+=htm(item)}else{console.log("unknown object",item)}}if(!emptyTags[obj.constructor.name]){buffer+="</"+obj.constructor.name+">"}return buffer}var html=htm(object);if(html&&object.constructor.name=="html"){return"<!DOCTYPE html>"+html}else{return html}};MarkConvert.toXml=function(object){function xml(obj){if(typeof obj!=="object"){return""}if(obj[$pragma]){return obj[$pragma].startsWith("!--")?"<"+obj[$pragma]+"--\x3e":"<?"+obj[$pragma]+"?>"}var buffer="<"+obj.constructor.name;for(var prop in obj){var value=obj[prop];if(value!=null){buffer+=" "+prop+'="'+escapeStr(value)+'"'}}var length=obj[$length];if(length){buffer+=">";for(var i=0;i<length;i++){var item=obj[i];if(typeof item==="string"){buffer+=escapeStr(item)}else if(typeof item==="object"){buffer+=xml(item)}else{console.log("unknown object",item)}}buffer+="</"+obj.constructor.name+">"}else{buffer+="/>"}return buffer}var result=xml(object);return'<?xml version="1.0" encoding="UTF-8"?>'+result};return MarkConvert};if(typeof module==="object")module.exports=MarkConvert},{htmlparser2:"htmlparser2"}]},{},[])("/lib/mark.convert.js")});