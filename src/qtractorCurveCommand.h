// qtractorCurveCommand.h
//
/****************************************************************************
   Copyright (C) 2005-2011, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __qtractorCurveCommand_h
#define __qtractorCurveCommand_h

#include "qtractorCommand.h"
#include "qtractorCurve.h"

#include <QList>


//----------------------------------------------------------------------
// class qtractorCurveBaseCommand - declaration.
//

class qtractorCurveBaseCommand : public qtractorCommand
{
public:

	// Constructor.
	qtractorCurveBaseCommand(const QString& sName);

	// Virtual command methods.
	bool redo();
	bool undo();

protected:

	// Common executive method.
	virtual bool execute(bool bRedo);
};


//----------------------------------------------------------------------
// class qtractorCurveCommand - declaration.
//

class qtractorCurveCommand : public qtractorCurveBaseCommand
{
public:

	// Constructor.
	qtractorCurveCommand(const QString& sName, qtractorCurve *pCurve);

protected:

	// Instance variables.
	qtractorCurve *m_pCurve;
};


//----------------------------------------------------------------------
// class qtractorCurveListCommand - declaration.
//

class qtractorCurveListCommand : public qtractorCurveBaseCommand
{
public:

	// Constructor.
	qtractorCurveListCommand(const QString& sName, qtractorCurveList *pCurveList);

protected:

	// Instance variables.
	qtractorCurveList *m_pCurveList;
};


//----------------------------------------------------------------------
// class qtractorCurveSelectCommand - declaration.
//

class qtractorCurveSelectCommand : public qtractorCurveListCommand
{
public:

	// Constructor.
	qtractorCurveSelectCommand(
		qtractorCurveList *pCurveList, qtractorCurve *pCurrentCurve);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	qtractorCurve *m_pCurrentCurve;
};


//----------------------------------------------------------------------
// class qtractorCurveModeCommand - declaration.
//

class qtractorCurveModeCommand : public qtractorCurveCommand
{
public:

	// Constructor.
	qtractorCurveModeCommand(
		qtractorCurve *pCurve, qtractorCurve::Mode mode);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	qtractorCurve::Mode m_mode;
};


//----------------------------------------------------------------------
// class qtractorCurveProcessCommand - declaration.
//

class qtractorCurveProcessCommand : public qtractorCurveCommand
{
public:

	// Constructor.
	qtractorCurveProcessCommand(
		qtractorCurve *pCurve, bool bProcess);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	bool m_bProcess;
};


//----------------------------------------------------------------------
// class qtractorCurveCaptureCommand - declaration.
//

class qtractorCurveCaptureCommand : public qtractorCurveCommand
{
public:

	// Constructor.
	qtractorCurveCaptureCommand(
		qtractorCurve *pCurve, bool bCapture);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	bool m_bCapture;
};


//----------------------------------------------------------------------
// class qtractorCurveLogarithmicCommand - declaration.
//

class qtractorCurveLogarithmicCommand : public qtractorCurveCommand
{
public:

	// Constructor.
	qtractorCurveLogarithmicCommand(
		qtractorCurve *pCurve, bool bLogarithmic);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	bool m_bLogarithmic;
};


//----------------------------------------------------------------------
// class qtractorCurveColorCommand - declaration.
//

class qtractorCurveColorCommand : public qtractorCurveCommand
{
public:

	// Constructor.
	qtractorCurveColorCommand(
		qtractorCurve *pCurve, const QColor& color);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	QColor m_color;
};


//----------------------------------------------------------------------
// class qtractorCurveProcessAllCommand - declaration.
//

class qtractorCurveProcessAllCommand : public qtractorCurveListCommand
{
public:

	// Constructor.
	qtractorCurveProcessAllCommand(
		qtractorCurveList *pCurveList, bool bProcessAll);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	bool m_bProcessAll;
};


//----------------------------------------------------------------------
// class qtractorCurveCaptureCommand - declaration.
//

class qtractorCurveCaptureAllCommand : public qtractorCurveListCommand
{
public:

	// Constructor.
	qtractorCurveCaptureAllCommand(
		qtractorCurveList *pCurveList, bool bCaptureAll);

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	bool m_bCaptureAll;
};


//----------------------------------------------------------------------
// class qtractorCurveEditCommand - declaration.
//

class qtractorCurveEditCommand : public qtractorCurveCommand
{
public:

	// Constructor.
	qtractorCurveEditCommand(
		const QString& sName, qtractorCurve *pCurve);
	qtractorCurveEditCommand(qtractorCurve *pCurve);

	// Destructor.
	virtual ~qtractorCurveEditCommand();

	// Primitive command methods.
	void addNode(qtractorCurve::Node *pNode);
	void removeNode(qtractorCurve::Node *pNode);

	// Composite predicate.
	bool isEmpty() const;

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Primitive command types.
	enum CommandType {
		AddNode, RemoveNode
	};

	// Curve item struct.
	struct Item
	{
		// Item constructor.
		Item(CommandType cmd, qtractorCurve::Node *pNode)
			: command(cmd), node(pNode), autoDelete(false) {}
		// Item members.
		CommandType command;
		qtractorCurve::Node *node;
		bool autoDelete;
	};

	// Instance variables.
	QList<Item *> m_items;
};


//----------------------------------------------------------------------
// class qtractorCurveClearCommand - declaration.
//

class qtractorCurveClearCommand : public qtractorCurveEditCommand
{
public:

	// Constructor.
	qtractorCurveClearCommand(qtractorCurve *pCurve);
};


//----------------------------------------------------------------------
// class qtractorCurveClearAllCommand - declaration.
//

class qtractorCurveClearAllCommand : public qtractorCurveListCommand
{
public:

	// Constructor.
	qtractorCurveClearAllCommand(qtractorCurveList *pCurveList);

	// Destructor.
	~qtractorCurveClearAllCommand();

	// Composite predicate.
	bool isEmpty() const;

protected:

	// Virtual executive method.
	bool execute(bool bRedo);

private:

	// Instance variables.
	QList<qtractorCurveClearCommand *> m_commands;
};


#endif	// __qtractorCurveCommand_h

// end of qtractorCurveCommand.h
