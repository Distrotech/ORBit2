/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 Andreas Kloeckner
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author:	Andreas Kloeckner <ak@ixion.net>
 *
 *  Purpose: Generic IDL compiler pass base class
 *
 */




#include "pass.hh"




// IDLPass --------------------------------------------------------------------
IDLPass::~IDLPass() {
	vector<IDLJob *>::const_iterator
		first = m_jobqueue.begin(),last = m_jobqueue.end();
	
	while (first != last) 
		delete *first++;
}




void IDLPass::runJobs(string const &event) {
	vector<IDLJob *>::iterator
		first = m_jobqueue.begin(),last = m_jobqueue.end();
	
	while (first != last) {
		if ((*first)->runForEvent(event)) {
			IDLJob *job = *first;
			job->run();
			first = m_jobqueue.erase(first);
			last = m_jobqueue.end();
			delete job;
		}
		else first++;
	}
}




// IDLOutputPass --------------------------------------------------------------
IDLOutputPass::~IDLOutputPass() {
	vector<IDLOutputJob *>::const_iterator
		first = m_outputjobqueue.begin(),last = m_outputjobqueue.end();
	
	while (first != last) 
		delete *first++;
}




void IDLOutputPass::runJobs(string const &event) {
	Super::runJobs(event);
	
	vector<IDLOutputJob *>::iterator
		first = m_outputjobqueue.begin(),last = m_outputjobqueue.end();
	
	while (first != last) {
		if ((*first)->runForEvent(event)) {
			IDLOutputJob *job = *first;
			job->run();
			first = m_outputjobqueue.erase(first);
			last = m_outputjobqueue.end();
			delete job;
		}
		else first++;
	}
}
