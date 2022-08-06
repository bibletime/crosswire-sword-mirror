/******************************************************************************
 *
 *  swconfig.cpp -	used for saving and retrieval of configuration
 *			information
 *
 * $Id$
 *
 * Copyright 1998-2013 CrossWire Bible Society (http://www.crosswire.org)
 *	CrossWire Bible Society
 *	P. O. Box 2528
 *	Tempe, AZ  85280-2528
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include <swconfig.h>
#include <utilstr.h>
#include <filemgr.h>
#include <fcntl.h>


SWORD_NAMESPACE_START

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

SWConfig::SWConfig() {
}


SWConfig::SWConfig(const char *ifilename) {
	filename = ifilename;
	load();
}


SWConfig::~SWConfig() {
}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

void SWConfig::load() {

	if (!getFileName().size()) return;	// assert we have a filename

	FileDesc *cfile;
	char *buf, *data;
	SWBuf line;
	ConfigEntMap curSect;
	SWBuf sectName;
	bool first = true;
	SWBuf commentLines;
	
	getSections().erase(getSections().begin(), getSections().end());
	
	cfile = FileMgr::getSystemFileMgr()->open(getFileName().c_str(), FileMgr::RDONLY);
	if (cfile->getFd() > 0) {
		bool goodLine = FileMgr::getLine(cfile, line);

		// clean UTF encoding tags at start of file
		while (goodLine && line.length() && 
				((((unsigned char)line[0]) == 0xEF) ||
				 (((unsigned char)line[0]) == 0xBB) ||
				 (((unsigned char)line[0]) == 0xBF))) {
			line << 1;
		}
		
		while (goodLine) {
			// ignore commented lines
			if (!line.startsWith("#")) {
				buf = new char [ line.length() + 1 ];
				strcpy(buf, line.c_str());
				if (*strstrip(buf) == '[') {
					if (!first) {
						getSections().insert(SectionMap::value_type(sectName, curSect));
					}
					else first = false;
					
					curSect.erase(curSect.begin(), curSect.end());

					strtok(buf, "]");
					sectName = buf+1;
					if (commentLines.size()) {
						getSections()["_ConfComments"][SWBuf("Section.") + sectName] = commentLines;
						commentLines = "";
					}
				}
				else {
					strtok(buf, "=");
					if ((*buf) && (*buf != '=')) {
						if ((data = strtok(NULL, ""))) {
							curSect.insert(ConfigEntMap::value_type(buf, strstrip(data)));
						}
						else curSect.insert(ConfigEntMap::value_type(buf, ""));
						if (commentLines.size()) {
							getSections()["_ConfComments"][SWBuf("Section.") + sectName + ".Entry." + buf] = commentLines;
							commentLines = "";
						}
					}
					else {
						if (commentLines.size() && !commentLines.endsWith("\n")) commentLines += "\n";
						commentLines += line;
					}
				}
				delete [] buf;
			}
			else {
				if (commentLines.size() && !commentLines.endsWith("\n")) commentLines += "\n";
				commentLines += line;
			}
			goodLine = FileMgr::getLine(cfile, line);
		}
		if (!first)
			getSections().insert(SectionMap::value_type(sectName, curSect));

		FileMgr::getSystemFileMgr()->close(cfile);
	}
}


void SWConfig::save() const {

	if (!getFileName().size()) return;	// assert we have a filename

	FileDesc *cfile;
	SWBuf buf;
	SectionMap::const_iterator sit;
	ConfigEntMap::const_iterator entry;
	SectionMap::const_iterator commentSection = getSections().find("_ConfComments");
	ConfigEntMap::const_iterator comment;
	SWBuf sectName;
	
	cfile = FileMgr::getSystemFileMgr()->open(getFileName().c_str(), FileMgr::RDWR|FileMgr::CREAT|FileMgr::TRUNC);
	if (cfile->getFd() > 0) {
		
		for (sit = getSections().begin(); sit != getSections().end(); ++sit) {
			buf =  "";
			sectName = (*sit).first;
			if (sectName == "_ConfComments") continue;
			if (commentSection != getSections().end()) {
				comment = (*commentSection).second.find(SWBuf("Section.") + sectName);
				if (comment != (*commentSection).second.end()) {
					if (!(*comment).second.startsWith("\n")) buf +=  SWBuf("\n");
					buf +=  (*comment).second;
				}
			}
			if (!buf.endsWith("\n")) buf += "\n";
			buf += "[";
			buf += (*sit).first.c_str();
			buf += "]\n";
			cfile->write(buf.c_str(), buf.length());
			for (entry = (*sit).second.begin(); entry != (*sit).second.end(); ++entry) {
				SWBuf entryKey = (*entry).first.c_str();
				buf =  "";
				if (commentSection != getSections().end()) {
					comment = (*commentSection).second.find(SWBuf("Section.") + sectName + ".Entry." + entryKey);
					if (comment != (*commentSection).second.end()) {
						buf += (*comment).second;
						if (!buf.endsWith("\n")) buf += "\n";
					}
				}
				buf += (*entry).first.c_str();
				buf += "=";
				buf += (*entry).second.c_str();
				buf += "\n";
				cfile->write(buf.c_str(), buf.length());
			}
		}
		buf = "\n";
		cfile->write(buf.c_str(), buf.length());
		FileMgr::getSystemFileMgr()->close(cfile);
	}
}


void SWConfig::augment(const SWConfig &addFrom) {

	SectionMap::const_iterator section;
	ConfigEntMap::const_iterator entry, start, end;

	for (section = addFrom.getSections().begin(); section != addFrom.getSections().end(); ++section) {
		for (entry = (*section).second.begin(); entry != (*section).second.end(); ++entry) {
			start = getSections()[section->first].lower_bound(entry->first);
			end   = getSections()[section->first].upper_bound(entry->first);
			// do we have multiple instances of the same key?
			if (start != end) {
				// TODO: what is this?
				ConfigEntMap::const_iterator x = addFrom.getSections().find(section->first)->second.lower_bound(entry->first);
				ConfigEntMap::const_iterator y = addFrom.getSections().find(section->first)->second.upper_bound(entry->first);
				++x;
				if (((++start) != end) || (x != y)) {
					for (--start; start != end; ++start) {
						if (!strcmp(start->second.c_str(), entry->second.c_str()))
							break;
					}
					if (start == end)
						getSections()[(*section).first].insert(ConfigEntMap::value_type((*entry).first, (*entry).second));
				}
				else	getSections()[section->first][entry->first.c_str()] = entry->second.c_str();
			}		
			else	getSections()[section->first][entry->first.c_str()] = entry->second.c_str();
		}
	}
}


#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// TODO: use deprecated public 'Sections' property for now until we remove deprecation
// and store in private property
SectionMap &SWConfig::getSections() { return Sections; }

// TODO: use deprecated public 'filename' property for now until we remove deprecation
// and store in private property
	
SWBuf SWConfig::getFileName() const { return filename; }

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

SWORD_NAMESPACE_END

