/*=========================================================================
*
*  Copyright RTK Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#include "srtkProcessObject.h"
#include "srtkCommand.h"

#include "itkProcessObject.h"
#include "itkCommand.h"
#include "itkImageToImageFilter.h"

#include <iostream>
#include <algorithm>

#include "nsstd/functional.h"

namespace rtk {
namespace simple {

namespace
{
static bool GlobalDefaultDebug = false;

static itk::AnyEvent eventAnyEvent;
static itk::AbortEvent eventAbortEvent;
static itk::DeleteEvent eventDeleteEvent;
static itk::EndEvent eventEndEvent;;
static itk::IterationEvent eventIterationEvent;
static itk::ProgressEvent eventProgressEvent;
static itk::StartEvent eventStartEvent;
static itk::UserEvent eventUserEvent;
static itk::MultiResolutionIterationEvent eventMultiResolutionIterationEvent;


// Local class to adapt a srtk::Command to ITK's command.
// It utilizes a raw pointer, and relies on the srtk
// ProcessObject<->Command reference to automatically remove it.
class SimpleAdaptorCommand
  : public itk::Command
{
public:

  typedef SimpleAdaptorCommand Self;
  typedef itk::SmartPointer< Self >  Pointer;

  itkNewMacro(Self);

  itkTypeMacro(SimpleAdaptorCommand, Command);

  void SetSimpleCommand( rtk::simple::Command *cmd )
    {
      m_That=cmd;
    }

  /**  Invoke the member function. */
  virtual void Execute(itk::Object *, const itk::EventObject &) SRTK_OVERRIDE
  {
    if (m_That)
      {
      m_That->Execute();
      }
  }

  /**  Invoke the member function with a const object */
  virtual void Execute(const itk::Object *, const itk::EventObject &) SRTK_OVERRIDE
  {
    if ( m_That )
      {
      m_That->Execute();
      }
  }

protected:
  rtk::simple::Command *                    m_That;
  SimpleAdaptorCommand():m_That(0) {}
  virtual ~SimpleAdaptorCommand() {}

private:
  SimpleAdaptorCommand(const Self &); //purposely not implemented
  void operator=(const Self &);        //purposely not implemented
};

} // end anonymous namespace

//----------------------------------------------------------------------------

//
// Default constructor that initializes parameters
//
ProcessObject::ProcessObject ()
  : m_Debug(ProcessObject::GetGlobalDefaultDebug()),
    m_NumberOfThreads(ProcessObject::GetGlobalDefaultNumberOfThreads()),
    m_ActiveProcess(NULL),
    m_ProgressMeasurement(0.0)
{
}


//
// Destructor
//
ProcessObject::~ProcessObject ()
{
  // ensure to remove reference between srtk commands and process object
  Self::RemoveAllCommands();
}

std::string ProcessObject::ToString() const
{
  std::ostringstream out;

  out << "  Debug: ";
  this->ToStringHelper(out, this->m_Debug) << std::endl;

  out << "  NumberOfThreads: ";
  this->ToStringHelper(out, this->m_NumberOfThreads) << std::endl;

  out << "  Commands:" << (m_Commands.empty()?" (none)":"") << std::endl;
  for( std::list<EventCommand>::const_iterator i = m_Commands.begin();
       i != m_Commands.end();
       ++i)
    {
    assert( i->m_Command );
    out << "    Event: " << i->m_Event << " Command: " << i->m_Command->GetName() << std::endl;
    }

  out << "  ProgressMeasurement: ";
  this->ToStringHelper(out, this->m_ProgressMeasurement) << std::endl;

  out << "  ActiveProcess:" << (this->m_ActiveProcess?"":" (none)") <<std::endl;
  if( this->m_ActiveProcess )
    {
    this->m_ActiveProcess->Print(out, itk::Indent(4));
    }

  return out.str();
}


std::ostream & ProcessObject::ToStringHelper(std::ostream &os, const char &v)
{
  os << int(v);
  return os;
}


std::ostream & ProcessObject::ToStringHelper(std::ostream &os, const signed char &v)
{
  os << int(v);
  return os;
}


std::ostream & ProcessObject::ToStringHelper(std::ostream &os, const unsigned char &v)
{
  os << int(v);
  return os;
}


void ProcessObject::DebugOn()
{
  this->m_Debug = true;
}


void ProcessObject::DebugOff()
{
  this->m_Debug = false;
}


bool ProcessObject::GetDebug() const
{
  return this->m_Debug;
}


void ProcessObject::SetDebug(bool debugFlag)
{
  this->m_Debug = debugFlag;
}


void ProcessObject::GlobalDefaultDebugOn()
{
  GlobalDefaultDebug = true;
}


void ProcessObject::GlobalDefaultDebugOff()

{
  GlobalDefaultDebug = false;
}


bool ProcessObject::GetGlobalDefaultDebug()
{
  return GlobalDefaultDebug;
}


void ProcessObject::SetGlobalDefaultDebug(bool debugFlag)
{
  GlobalDefaultDebug = debugFlag;
}


void ProcessObject::GlobalWarningDisplayOn()
{
  itk::Object::GlobalWarningDisplayOn();
}


void ProcessObject::GlobalWarningDisplayOff()
{
  itk::Object::GlobalWarningDisplayOff();
}


bool ProcessObject::GetGlobalWarningDisplay()
{
  return itk::Object::GetGlobalWarningDisplay();
}


void ProcessObject::SetGlobalWarningDisplay(bool flag)
{
  itk::Object::SetGlobalWarningDisplay(flag);
}


double ProcessObject::GetGlobalDefaultCoordinateTolerance()
{
  return itk::ImageToImageFilterCommon::GetGlobalDefaultCoordinateTolerance();
}

void ProcessObject::SetGlobalDefaultCoordinateTolerance(double tolerance)
{
  return itk::ImageToImageFilterCommon::SetGlobalDefaultCoordinateTolerance(tolerance);
}

double ProcessObject::GetGlobalDefaultDirectionTolerance()
{
  return itk::ImageToImageFilterCommon::GetGlobalDefaultDirectionTolerance();
}

void ProcessObject::SetGlobalDefaultDirectionTolerance(double tolerance)
{
  return itk::ImageToImageFilterCommon::SetGlobalDefaultDirectionTolerance(tolerance);
}


void ProcessObject::SetGlobalDefaultNumberOfThreads(unsigned int n)
{
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(n);
}


unsigned int ProcessObject::GetGlobalDefaultNumberOfThreads()
{
  return itk::MultiThreader::GetGlobalDefaultNumberOfThreads();
}


void ProcessObject::SetNumberOfThreads(unsigned int n)
{
  m_NumberOfThreads = n;
}


unsigned int ProcessObject::GetNumberOfThreads() const
{
  return m_NumberOfThreads;
}


int ProcessObject::AddCommand(EventEnum event, Command &cmd)
{
  // add to our list of event, command pairs
  m_Commands.push_back(EventCommand(event,&cmd));

  // register ourselves with the command
  cmd.AddProcessObject(this);

  if (this->m_ActiveProcess)
    {
    this->AddObserverToActiveProcessObject( m_Commands.back() );
    }
  else
    {
    m_Commands.back().m_ITKTag = std::numeric_limits<unsigned long>::max();
    }

  return 0;
}


void ProcessObject::RemoveAllCommands()
{
  // set's the m_Commands to an empty list via a swap
  std::list<EventCommand> oldCommands;
  swap(oldCommands, m_Commands);

  // remove commands from active process object
  std::list<EventCommand>::iterator i = oldCommands.begin();
  while( i != oldCommands.end() && this->m_ActiveProcess )
    {
    this->RemoveObserverFromActiveProcessObject(*i);
    ++i;
    }

  // we must only call RemoveProcessObject once for each command
  // so make a unique list of the Commands.
  oldCommands.sort();
  oldCommands.unique();
  i = oldCommands.begin();
  while( i != oldCommands.end() )
    {
    // note: this may call onCommandDelete, but we have already copied
    // this->m_Command will be empty
    i++->m_Command->RemoveProcessObject(this);
    }
}


bool ProcessObject::HasCommand( EventEnum event ) const
{
  std::list<EventCommand>::const_iterator i = m_Commands.begin();
  while( i != m_Commands.end() )
    {
    if (i->m_Event == event)
      {
      return true;
      }
    ++i;
    }
  return false;
}


float ProcessObject::GetProgress( ) const
{
  if ( this->m_ActiveProcess )
    {
    return this->m_ActiveProcess->GetProgress();
    }
  return m_ProgressMeasurement;
}


void ProcessObject::Abort()
{
  if ( this->m_ActiveProcess )
    {
    this->m_ActiveProcess->AbortGenerateDataOn();
    }
}


void ProcessObject::PreUpdate(itk::ProcessObject *p)
{
  assert(p);

  // propagate number of threads
  p->SetNumberOfThreads(this->GetNumberOfThreads());

  try
    {
    this->m_ActiveProcess = p;

    // add command on active process deletion
    itk::SimpleMemberCommand<Self>::Pointer onDelete = itk::SimpleMemberCommand<Self>::New();
    onDelete->SetCallbackFunction(this, &Self::OnActiveProcessDelete);
    p->AddObserver(itk::DeleteEvent(), onDelete);

    // register commands
    for (std::list<EventCommand>::iterator i = m_Commands.begin();
         i != m_Commands.end();
         ++i)
      {
      this->AddObserverToActiveProcessObject(*i);
      }

    }
  catch (...)
    {
    this->m_ActiveProcess = NULL;
    throw;
    }

  if (this->GetDebug())
     {
     std::cout << "Executing ITK filter:" << std::endl;
     p->Print(std::cout);
     }
}


unsigned long ProcessObject::AddITKObserver( const itk::EventObject &e,
                                             itk::Command *c)
{
  assert(this->m_ActiveProcess);
  return this->m_ActiveProcess->AddObserver(e,c);
}


void ProcessObject::RemoveITKObserver( EventCommand &e )
{
  assert(this->m_ActiveProcess);
  this->m_ActiveProcess->RemoveObserver(e.m_ITKTag);
}



const itk::EventObject &ProcessObject::GetITKEventObject(EventEnum e)
{
  switch (e)
    {
    case srtkAnyEvent:
      return eventAnyEvent;
    case srtkAbortEvent:
      return eventAbortEvent;
    case srtkDeleteEvent:
      return eventDeleteEvent;
    case srtkEndEvent:
      return eventEndEvent;
    case srtkIterationEvent:
      return eventIterationEvent;
    case srtkProgressEvent:
      return eventProgressEvent;
    case srtkStartEvent:
      return eventStartEvent;
    case srtkUserEvent:
      return eventUserEvent;
    case srtkMultiResolutionIterationEvent:
      return eventMultiResolutionIterationEvent;
    default:
      srtkExceptionMacro("LogicError: Unexpected event case!");
    }
}


itk::ProcessObject *ProcessObject::GetActiveProcess( )
{
  if (this->m_ActiveProcess)
    {
    return this->m_ActiveProcess;
    }
  srtkExceptionMacro("No active process for \"" << this->GetName() << "\"!");
}


void ProcessObject::OnActiveProcessDelete( )
{
  if (this->m_ActiveProcess)
    {
    this->m_ProgressMeasurement = this->m_ActiveProcess->GetProgress();
    }
  else
    {
    this->m_ProgressMeasurement = 0.0f;
    }

  // clear registered command IDs
  for (std::list<EventCommand>::iterator i = m_Commands.begin();
         i != m_Commands.end();
         ++i)
      {
      i->m_ITKTag = std::numeric_limits<unsigned long>::max();
      }

  this->m_ActiveProcess = NULL;
}


void ProcessObject::onCommandDelete(const rtk::simple::Command *cmd) throw()
{
  // remove command from m_Command book keeping list, and remove it
  // from the  ITK ProcessObject
  std::list<EventCommand>::iterator i =  this->m_Commands.begin();
  while ( i !=  this->m_Commands.end() )
    {
    if ( cmd == i->m_Command )
      {
      if ( this->m_ActiveProcess )
        {
        this->RemoveObserverFromActiveProcessObject( *i );
        }
      this->m_Commands.erase(i++);
      }
    else
      {
      ++i;
      }

    }
}

unsigned long ProcessObject::AddObserverToActiveProcessObject( EventCommand &eventCommand )
{
  assert( this->m_ActiveProcess );

  if (eventCommand.m_ITKTag != std::numeric_limits<unsigned long>::max())
    {
    srtkExceptionMacro("Commands already registered to another process object!");
    }

  const itk::EventObject &itkEvent = GetITKEventObject(eventCommand.m_Event);

  // adapt srtk command to itk command
  SimpleAdaptorCommand::Pointer itkCommand = SimpleAdaptorCommand::New();
  itkCommand->SetSimpleCommand(eventCommand.m_Command);
  itkCommand->SetObjectName(eventCommand.m_Command->GetName()+" "+itkEvent.GetEventName());

  return eventCommand.m_ITKTag = this->AddITKObserver( itkEvent, itkCommand );
}

void ProcessObject::RemoveObserverFromActiveProcessObject( EventCommand &e )
 {
   assert( this->m_ActiveProcess );

   if (e.m_ITKTag != std::numeric_limits<unsigned long>::max() )
     {
     this->RemoveITKObserver(e);
     e.m_ITKTag = std::numeric_limits<unsigned long>::max();
     }

 }

} // end namespace simple
} // end namespace itk
