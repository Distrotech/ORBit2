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
 *			John K. Luebs <jkluebs@marikarpress.com>
 *
 *  Purpose: Generic IDL compiler pass base class
 *
 */
 



#ifndef ORBITCPP_PASS
#define ORBITCPP_PASS



#include "base.hh"
#include "types.hh"
#include "types/IDLInterface.hh"
//#include "types/IDLSequenceList.hh"
#include "types/IDLArrayList.hh"
#include <iostream>
#include <vector>





#define IDL_EV_TOPLEVEL			"toplevel"




class IDLPass;
class IDLOutputPass;


class IDLCompilerState {
public:
	string                  m_basename;
	IDLScope                m_rootscope;
	vector<IDLInterface *>  m_interfaces;
	IDLTypeParser           m_typeparser;
//	IDLSequenceList         m_seqs;
	IDLArrayList            m_arrays;

	IDLPass                *m_pass_gather;
	IDLOutputPass          *m_pass_xlate;
	IDLOutputPass          *m_pass_stubs;
	IDLOutputPass          *m_pass_skels;

	IDLCompilerState(string const &basename,IDL_tree list)
		: m_basename(basename),m_rootscope("",list),m_typeparser(*this) {
	}
};




class IDLPass {
protected:
	IDLCompilerState			&m_state;
	class IDLJob;
	vector<IDLJob *>			m_jobqueue;
	
public:
	class IDLJob {
	protected:
		string						m_event;
		IDLCompilerState			&m_state;

		IDLJob(string const &event,IDLCompilerState &state)
			: m_event(event),m_state(state) {
		}
		
	public:
		IDLJob(string const &event,IDLCompilerState &state,IDLPass &pass)
			: m_event(event),m_state(state) {
			pass.queueJob(this);
		}
		virtual ~IDLJob() {
		}
		virtual bool runForEvent(string const &event) {
			return m_event == event || event == "";
		}
		virtual void run() = 0;
	};
	friend class IDLJob;
	
	IDLPass(IDLCompilerState &state)
		: m_state(state) {
	}
	virtual ~IDLPass();
	
	virtual void runJobs(string const &event = "");
	virtual void runPass() = 0;

protected:
	void queueJob(IDLJob *job) {
		m_jobqueue.push_back(job);
	}
};




class IDLOutputPass : public IDLPass {
	typedef	IDLPass					Super;
public:
	class IDLOutputJob : public IDLJob {
	protected:
		ostream						&m_header;
		ostream						&m_module;
		Indent						&indent;
		Indent						&mod_indent;
	public:
		IDLOutputJob(string const &event,IDLCompilerState &state,IDLOutputPass &pass)
			: IDLJob(event,state),m_header(pass.m_header),m_module(pass.m_module),
			indent(pass.indent),mod_indent(pass.mod_indent) {
			pass.queueJob(this);
		}
	};
	friend class IDLOutputJob;
	
	IDLOutputPass(IDLCompilerState &state,ostream &header,ostream &module)
		: Super(state),m_header(header),m_module(module) {
	}
	~IDLOutputPass();
	
	void runJobs(string const &event = "");

protected:
	ostream							&m_header;
	ostream							&m_module;
	Indent							indent;
	Indent							mod_indent;
	vector<IDLOutputJob *>			m_outputjobqueue;

	void queueJob(IDLJob *job) {
		Super::queueJob(job);
	}
	void queueJob(IDLOutputJob *job) {
		m_outputjobqueue.push_back(job);
	}
};




#endif

