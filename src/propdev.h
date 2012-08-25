/////////////////////////////////////////////////////////////////////////////
// Name:        propdev.h
// Purpose:     wxPropertyGrid Internal/Property Developer Header
// Author:      Jaakko Salli
// Modified by:
// Created:     Nov-23-2004
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_PROPDEV_H_
#define _WX_PROPGRID_PROPDEV_H_

// -----------------------------------------------------------------------

#ifdef _WX_PROPGRID_ADVPROPS_H_
    #error "propdev.h must be included *before* advprops.h"
#endif


class WXDLLIMPEXP_PG wxArrayEditorDialog;

// -----------------------------------------------------------------------

// These are intuitive substitutes for base property classes.
typedef wxPGProperty                wxBasePropertyClass;
typedef wxPGPropertyWithChildren    wxBaseParentPropertyClass;

// -----------------------------------------------------------------------

#ifndef SWIG

// This is required for sharing common global variables.
// TODO: Automatic locking mechanism?
class WXDLLIMPEXP_PG wxPGGlobalVarsClass
{
public:

    wxPGGlobalVarsClass();
    ~wxPGGlobalVarsClass();

    wxString            m_pDefaultImageWildcard; // Used by advprops, but here to make things easier.

    //wxArrayPtrVoid      m_arrEditorClasses; // List of editor class instances.
    wxPGHashMapS2P      m_mapEditorClasses; // Map of editor class instances (keys are name string).

#if wxUSE_VALIDATORS
    wxArrayPtrVoid      m_arrValidators; // These wxValidators need to be freed
#endif

    wxPGHashMapS2P      m_dictValueType; // Valuename -> Value type object instance.

#if wxPG_VALUETYPE_IS_STRING
    wxPGHashMapS2P      m_dictValueTypeByClass; // VTClassname -> Value type object instance.
#endif

    wxPGHashMapS2P      m_dictPropertyClassInfo; // PropertyName -> ClassInfo

    wxPGChoices*        m_fontFamilyChoices;

    int                 m_numBoolChoices;  // 2 or 3

    wxString            m_boolChoices[3]; // default is ["False", "True"]. Extra space is for "Unspecified".

    bool                m_autoGetTranslation; // If true then some things are automatically translated

    int                 m_offline; // > 0 if errors cannot or should not be shown in statusbar etc.

/*#if wxUSE_THREADS
    wxCriticalSection   m_critSect; // To allow different property grids to be addressed from different threads
#endif*/
};

extern WXDLLIMPEXP_PG wxPGGlobalVarsClass* wxPGGlobalVars;

#endif


/*
#define wxPGUnRefChoices(PCHOICESDATA) \
    PCHOICESDATA->UnRef()
*/

// TODO: This locks on Linux, fix it!
/*#if wxUSE_THREADS
    #define WX_PG_GLOBALS_LOCKER() \
        wxCriticalSectionLocker _globalVarsLocker_(wxPGGlobalVars->m_critSect);
#else*/
    #define WX_PG_GLOBALS_LOCKER()
//#endif

// -----------------------------------------------------------------------
// wxPGEditor class.


// Needed for wxPython bindings
class wxPGWindowPair
{
public:
    wxPGWindowPair()
    {
        m_primary = m_secondary = NULL;
    }

    wxWindow*   m_primary;
    wxWindow*   m_secondary;

#ifndef SWIG
    wxPGWindowPair( wxWindow* a )
    {
        m_primary = a;
        m_secondary = NULL;
    };
    wxPGWindowPair( wxWindow* a, wxWindow* b )
    {
        m_primary = a;
        m_secondary = b;
    };
#endif
};


/** \class wxPGEditor
    \ingroup classes
    \brief Base for property editor classes.
    \remarks
    - Event handling:
      wxPGEditor::CreateControls should Connect all necessary events to the
      wxPropertyGrid::OnCustomEditorEvent. For Example:
        \code
            // Relays wxEVT_COMMAND_TEXT_UPDATED events of primary editor
            // control to the OnEvent.
            // NOTE: This event in particular is actually automatically conveyed, but
            //   it is just used as an example.
            propgrid->Connect( wxPG_SUBID1, wxEVT_COMMAND_TEXT_UPDATED,
                              (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                              &wxPropertyGrid::OnCustomEditorEvent );
        \endcode
      OnCustomEditorEvent will then forward events, first to wxPGEditor::OnEvent
      and then to wxPGProperty::OnEvent.
    - You need to call macro wxPGRegisterEditorClass(EditorName) before using a custom editor class.
    - See propgrid.cpp for how builtin editors works (starting from wxPGTextCtrlEditor).
*/
class WXDLLIMPEXP_PG wxPGEditor
{
public:

    /** Constructor. */
    wxPGEditor()
    {
    #if defined(__WXPYTHON__)
        m_scriptObject = NULL;
    #endif
    }

    /** Destructor. */
    virtual ~wxPGEditor();

    /** Returns pointer to the name of the editor. For example, wxPG_EDITOR(TextCtrl)
        has name "TextCtrl". This method is autogenerated for custom editors.
    */
    virtual wxPG_CONST_WXCHAR_PTR GetName() const = 0;

    /** Instantiates editor controls.
        \remarks
        - Primary control shall use id wxPG_SUBID1, and secondary (button) control
          shall use wxPG_SUBID2.
        \param propgrid
        wxPropertyGrid to which the property belongs (use as parent for control).
        \param propert
        Property for which this method is called.
        \param pos
        Position, inside wxPropertyGrid, to create control(s) to.
        \param size
        Initial size for control(s).
        \param psecondary
        If method generates a secondary (button) control, pointer to it must
        be stored here.
    */
#ifndef __WXPYTHON__
    virtual wxWindow* CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property,
        const wxPoint& pos, const wxSize& sz, wxWindow** psecondary ) const = 0;
    #define wxPG_DECLARE_CREATECONTROLS \
        virtual wxWindow* CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property, \
        const wxPoint& pos, const wxSize& sz, wxWindow** psecondary ) const;
#else
    virtual wxPGWindowPair CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property,
        const wxPoint& pos, const wxSize& sz ) const = 0;
    #define wxPG_DECLARE_CREATECONTROLS \
        virtual wxPGWindowPair CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property, \
        const wxPoint& pos, const wxSize& sz ) const;
#endif

    /** Loads value from property to the control. */
    virtual void UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const = 0;

    /** Used to draw the value when control is hidden. Default is to draw a string.
        Note that some margin above and below has been taken into account, to make
        drawing text easier without giving loads parameters. If Pen is changed,
        it must be returned to *wxTRANSPARENT_PEN.
    */
    virtual void DrawValue( wxDC& dc, wxPGProperty* property, const wxRect& rect ) const;

    /** Handles events. Returns true if value in control was modified
        (see wxPGProperty::OnEvent for more information).
    */
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd_primary, wxEvent& event ) const = 0;

    /** Copies value from ctrl to property's internal storage.
        Returns true if value was different.
    */
    virtual bool CopyValueFromControl( wxPGProperty* property, wxWindow* ctrl ) const = 0;

    /** Sets value in control to unspecified. */
    virtual void SetValueToUnspecified( wxWindow* ctrl ) const = 0;

    /** Sets control's value specifically from string. */
    virtual void SetControlStringValue( wxWindow* ctrl, const wxString& txt ) const;

    /** Sets control's value specifically from int (applies to choice etc.). */
    virtual void SetControlIntValue( wxWindow* ctrl, int value ) const;

    /** Inserts item to existing control. Index -1 means appending.
        Default implementation does nothing. Returns index of item added.
    */
    virtual int InsertItem( wxWindow* ctrl, const wxString& label, int index ) const;

    /** Deletes item from existing control.
        Default implementation does nothing.
    */
    virtual void DeleteItem( wxWindow* ctrl, int index ) const;

    /** Extra processing when control gains focus. For example, wxTextCtrl 
        based controls should select all text.
    */
    virtual void OnFocus( wxPGProperty* property, wxWindow* wnd ) const;

    /** Returns true if control itself can contain the custom image. Default is
        to return false.
    */
    virtual bool CanContainCustomImage() const;

#if defined(__WXPYTHON__) && !defined(SWIG)
    // This is the python object that contains and owns the C++ representation.
    PyObject*                   m_scriptObject;
#endif

protected:
};


//#ifndef SWIG
#if 1


#define WX_PG_DECLARE_EDITOR_CLASS() \
public: \
    virtual wxPG_CONST_WXCHAR_PTR GetName() const; \
private:


#define WX_PG_IMPLEMENT_EDITOR_CLASS(EDITOR,CLASSNAME,BASECLASS) \
wxPG_CONST_WXCHAR_PTR CLASSNAME::GetName() const \
{ \
    return wxT(#EDITOR); \
} \
wxPGEditor* wxPGEditor_##EDITOR = (wxPGEditor*) NULL; \
wxPGEditor* wxPGConstruct##EDITOR##EditorClass() \
{ \
    wxASSERT( !wxPGEditor_##EDITOR ); \
    return new CLASSNAME(); \
}


#define WX_PG_IMPLEMENT_EDITOR_CLASS_STD_METHODS() \
wxPG_DECLARE_CREATECONTROLS \
virtual void UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const; \
virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property, \
    wxWindow* primary, wxEvent& event ) const; \
virtual bool CopyValueFromControl( wxPGProperty* property, wxWindow* ctrl ) const; \
virtual void SetValueToUnspecified( wxWindow* ctrl ) const;


//
// Following are the built-in editor classes.
//

class WXDLLIMPEXP_PG wxPGTextCtrlEditor : public wxPGEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    wxPGTextCtrlEditor() {}
    virtual ~wxPGTextCtrlEditor();

    WX_PG_IMPLEMENT_EDITOR_CLASS_STD_METHODS()

    virtual void DrawValue( wxDC& dc, wxPGProperty* property, const wxRect& rect ) const;
    virtual void SetControlStringValue( wxWindow* ctrl, const wxString& txt ) const;
    virtual void OnFocus( wxPGProperty* property, wxWindow* wnd ) const;

    // Provided so that, for example, ComboBox editor can use the same code
    // (multiple inheritance would get way too messy).
    static bool OnTextCtrlEvent( wxPropertyGrid* propgrid,
                                 wxPGProperty* property,
                                 wxWindow* ctrl,
                                 wxEvent& event );

    static bool CopyTextCtrlValueFromControl( wxPGProperty* property, wxWindow* ctrl );

};


class WXDLLIMPEXP_PG wxPGChoiceEditor : public wxPGEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    wxPGChoiceEditor() {}
    virtual ~wxPGChoiceEditor();

    WX_PG_IMPLEMENT_EDITOR_CLASS_STD_METHODS()

    virtual void SetControlIntValue( wxWindow* ctrl, int value ) const;
    virtual void SetControlStringValue( wxWindow* ctrl, const wxString& txt ) const;

    virtual int InsertItem( wxWindow* ctrl, const wxString& label, int index ) const;
    virtual void DeleteItem( wxWindow* ctrl, int index ) const;
    virtual bool CanContainCustomImage() const;

    // CreateControls calls this with CB_READONLY in extraStyle
    wxWindow* CreateControlsBase( wxPropertyGrid* propgrid,
                                  wxPGProperty* property,
                                  const wxPoint& pos,
                                  const wxSize& sz,
                                  long extraStyle ) const;

};


class WXDLLIMPEXP_PG wxPGComboBoxEditor : public wxPGChoiceEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    wxPGComboBoxEditor() {}
    virtual ~wxPGComboBoxEditor();

    wxPG_DECLARE_CREATECONTROLS  // Macro is used for conviency due to different signature with wxPython

    virtual void UpdateControl( wxPGProperty* property, wxWindow* ctrl ) const;

    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* ctrl, wxEvent& event ) const;

    virtual bool CopyValueFromControl( wxPGProperty* property, wxWindow* ctrl ) const;

    virtual void OnFocus( wxPGProperty* property, wxWindow* wnd ) const;

};


class WXDLLIMPEXP_PG wxPGChoiceAndButtonEditor : public wxPGChoiceEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    wxPGChoiceAndButtonEditor() {}
    virtual ~wxPGChoiceAndButtonEditor();
    wxPG_DECLARE_CREATECONTROLS  // Macro is used for conviency due to different signature with wxPython
};


class WXDLLIMPEXP_PG wxPGTextCtrlAndButtonEditor : public wxPGTextCtrlEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    wxPGTextCtrlAndButtonEditor() {}
    virtual ~wxPGTextCtrlAndButtonEditor();
    wxPG_DECLARE_CREATECONTROLS
};


#if wxPG_INCLUDE_CHECKBOX

//
// Use custom check box code instead of native control
// for cleaner (ie. more integrated) look.
//
class WXDLLIMPEXP_PG wxPGCheckBoxEditor : public wxPGEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    wxPGCheckBoxEditor() {}
    virtual ~wxPGCheckBoxEditor();

    WX_PG_IMPLEMENT_EDITOR_CLASS_STD_METHODS()

    virtual void DrawValue( wxDC& dc, wxPGProperty* property, const wxRect& rect ) const;

    virtual void SetControlIntValue( wxWindow* ctrl, int value ) const;
};

#endif

#endif // SWIG


// -----------------------------------------------------------------------
// Value type registeration macros

#define wxPGRegisterValueType(TYPENAME) \
    if ( wxPGValueType_##TYPENAME == (wxPGValueType*) NULL ) \
    { \
        wxPGValueType_##TYPENAME = wxPropertyGrid::RegisterValueType( wxPGNewVT##TYPENAME(), false, wxT(#TYPENAME) ); \
    }

// Use this in RegisterDefaultValues.
#define wxPGRegisterDefaultValueType(TYPENAME) \
    if ( wxPGValueType_##TYPENAME == (wxPGValueType*) NULL ) \
    { \
        wxPGValueType_##TYPENAME = wxPropertyGrid::RegisterValueType( new wxPGValueType##TYPENAME##Class, true, wxT(#TYPENAME) ); \
    }

#define wxPG_INIT_REQUIRED_TYPE(T) \
    wxPGRegisterValueType(T)

// Use this with 'simple' value types (derived)
#define wxPG_INIT_REQUIRED_TYPE2(T) \
    if ( wxPGValueType_##T == (wxPGValueType*) NULL ) \
    { \
        wxPGValueType_##T = wxPropertyGrid::RegisterValueType( new wxPGValueType##T##Class, false, wxT(#T) ); \
    }

// -----------------------------------------------------------------------
// Editor class registeration macros

#define wxPGRegisterEditorClass(EDITOR) \
    if ( wxPGEditor_##EDITOR == (wxPGEditor*) NULL ) \
    { \
        wxPGEditor_##EDITOR = wxPropertyGrid::RegisterEditorClass( wxPGConstruct##EDITOR##EditorClass(), wxT(#EDITOR) ); \
    }

// Use this in RegisterDefaultEditors.
#define wxPGRegisterDefaultEditorClass(EDITOR) \
if ( wxPGEditor_##EDITOR == (wxPGEditor*) NULL ) \
    { \
        wxPGEditor_##EDITOR = wxPropertyGrid::RegisterEditorClass( wxPGConstruct##EDITOR##EditorClass(), wxT(#EDITOR), true ); \
    }

#define wxPG_INIT_REQUIRED_EDITOR(T) \
    wxPGRegisterEditorClass(T)

// -----------------------------------------------------------------------

#define WX_PG_IMPLEMENT_SUBTYPE(VALUETYPE,CVALUETYPE,DEFPROPERTY,TYPESTRING,GETTER,DEFVAL) \
const wxPGValueType *wxPGValueType_##VALUETYPE = (wxPGValueType*) NULL; \
class wxPGValueType##VALUETYPE##Class : public wxPGValueType \
{ \
public: \
    virtual wxPG_CONST_WXCHAR_PTR GetTypeName() const { return TYPESTRING; } \
    virtual wxPGVariant GetDefaultValue() const { return wxPGVariantCreator(m_default); } \
    virtual wxVariant GenerateVariant( wxPGVariant value, const wxString& name ) const \
    { return wxVariant( value.GETTER(), name ); } \
    virtual wxPGProperty* GenerateProperty( const wxString& label, const wxString& name ) const \
    { \
        return wxPG_CONSTFUNC(DEFPROPERTY)(label,name); \
    } \
    virtual void SetValueFromVariant( wxPGProperty* property, wxVariant& value ) const \
    { \
        wxPG_CHECK_RET_DBG( wxStrcmp(GetTypeName(),value.GetType().c_str()) == 0, \
            wxT("SetValueFromVariant: wxVariant type mismatch.") ); \
        property->DoSetValue(value.GETTER()); \
    } \
    wxPGValueType##VALUETYPE##Class(); \
    virtual ~wxPGValueType##VALUETYPE##Class(); \
protected: \
    CVALUETYPE m_default; \
}; \
wxPGValueType##VALUETYPE##Class::wxPGValueType##VALUETYPE##Class() { m_default = DEFVAL; } \
wxPGValueType##VALUETYPE##Class::~wxPGValueType##VALUETYPE##Class() { }

#define WX_PG_IMPLEMENT_VALUE_TYPE(VALUETYPE,DEFPROPERTY,TYPESTRING,GETTER,DEFVAL) \
WX_PG_IMPLEMENT_SUBTYPE(VALUETYPE,VALUETYPE,DEFPROPERTY,TYPESTRING,GETTER,DEFVAL)

//
// Implements wxVariantData for the type.
//
#define WX_PG_IMPLEMENT_VALUE_TYPE_VDC(VDCLASS,VALUETYPE) \
WX_PG_IMPLEMENT_DYNAMIC_CLASS_VARIANTDATA(VDCLASS,wxVariantData) \
VDCLASS::VDCLASS() { } \
VDCLASS::VDCLASS(const VALUETYPE& value) \
{ \
    m_value = value; \
} \
void VDCLASS::Copy(wxVariantData& data) \
{ \
    wxPG_CHECK_RET_DBG( data.GetType() == GetType(), wxT(#VDCLASS) wxT("::Copy: Can't copy to this type of data") ); \
    VDCLASS& otherData = (VDCLASS&) data; \
    otherData.m_value = m_value; \
} \
wxString VDCLASS::GetType() const \
{ \
    return wxString(wxT(#VALUETYPE)); \
} \
bool VDCLASS::Eq(wxVariantData& data) const \
{ \
    wxPG_CHECK_MSG_DBG( data.GetType() == GetType(), false, wxT(#VDCLASS) wxT("::Eq: argument mismatch") ); \
    VDCLASS& otherData = (VDCLASS&) data; \
    return otherData.m_value == m_value; \
} \
void* VDCLASS::GetValuePtr() { return (void*)&m_value; }


#define WX_PG_GENVARIANT_WXOBJ_BASE(VALUETYPE) \
virtual wxVariant GenerateVariant( wxPGVariant value, const wxString& name ) const \
{ return wxVariant( new wxVariantData_##VALUETYPE( (*(VALUETYPE*)wxPGVariantGetWxObjectPtr(value)) ), name ); }

#define WX_PG_GENVARIANT_VOIDP_SIMPLE() \
virtual wxVariant GenerateVariant( wxPGVariant value, const wxString& name ) const \
{ \
    void* ptr = (void*)wxPGVariantToVoidPtr(value); \
    wxASSERT( ptr ); \
    if ( !ptr ) return wxVariant(); \
    return wxVariant( ptr, name ); \
}

#define WX_PG_GENVARIANT_VOIDP_CVD(VDCLASS, VALUETYPE) \
virtual wxVariant GenerateVariant( wxPGVariant value, const wxString& name ) const \
{ \
    void* ptr = (void*)wxPGVariantToVoidPtr(value); \
    wxASSERT( ptr ); \
    if ( !ptr ) return wxVariant(); \
    return wxVariant( new VDCLASS(*((VALUETYPE*)ptr)), name ); \
}


// TODO: When ready, remove GetValueClassInfo
#define WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ_BASE(VALUETYPE,DEFPROPERTY,DEFVAL) \
class WX_PG_DECLARE_VALUE_TYPE_VDC(VALUETYPE) \
    virtual wxClassInfo* GetValueClassInfo(); \
}; \
WX_PG_IMPLEMENT_VALUE_TYPE_VDC(wxVariantData_##VALUETYPE,VALUETYPE) \
wxClassInfo* wxVariantData_##VALUETYPE::GetValueClassInfo() \
{ \
    return m_value.GetClassInfo(); \
} \
const wxPGValueType *wxPGValueType_##VALUETYPE = (wxPGValueType*) NULL; \
class wxPGValueType##VALUETYPE##Class : public wxPGValueType \
{ \
public: \
    virtual wxPG_CONST_WXCHAR_PTR GetTypeName() const { return CLASSINFO(VALUETYPE)->GetClassName(); } \
    WX_PG_GENVARIANT_WXOBJ_BASE(VALUETYPE) \
    virtual wxPGProperty* GenerateProperty( const wxString& label, const wxString& name ) const \
    { \
        return wxPG_CONSTFUNC(DEFPROPERTY)(label,name); \
    } \
    virtual void SetValueFromVariant( wxPGProperty* property, wxVariant& value ) const \
    { \
        const VALUETYPE* real_value; \
        wxPG_CHECK_RET_DBG( wxStrcmp(GetTypeName(),value.GetType().c_str()) == 0, \
            wxT("GetPtrFromVariant: wxVariant type mismatch.") ); \
        wxVariantData_##VALUETYPE* vd = wxDynamicCastVariantData(value.GetData(), wxVariantData_##VALUETYPE); \
        if ( vd ) \
            real_value = &vd->GetValue(); \
        else \
            real_value  = ((const VALUETYPE*)value.GetWxObjectPtr()); \
        property->DoSetValue( (wxObject*) real_value ); \
    }

// This should not be used by built-in types (advprops.cpp types should use it though)
#define WX_PG_IMPLEMENT_VALUE_TYPE_CREATOR(VALUETYPE) \
wxPGValueType* wxPGNewVT##VALUETYPE() { return new wxPGValueType##VALUETYPE##Class; }

#define WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ(VALUETYPE,DEFPROPERTY,DEFVAL) \
WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ_BASE(VALUETYPE,DEFPROPERTY,DEFVAL) \
    virtual wxPGVariant GetDefaultValue() const { return wxPGVariantCreator(DEFVAL); } \
}; \
WX_PG_IMPLEMENT_VALUE_TYPE_CREATOR(VALUETYPE)

#define WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ_OWNDEFAULT(VALUETYPE,DEFPROPERTY,DEFVAL) \
WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ_BASE(VALUETYPE,DEFPROPERTY,DEFVAL) \
    wxPGValueType##VALUETYPE##Class() { m_default = DEFVAL; } \
    virtual ~wxPGValueType##VALUETYPE##Class() { } \
    virtual wxPGVariant GetDefaultValue() const { return wxPGVariantCreator((wxObject*)&m_default); } \
protected: \
    VALUETYPE   m_default; \
}; \
WX_PG_IMPLEMENT_VALUE_TYPE_CREATOR(VALUETYPE)


#define WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_BASE(VALUETYPE,DEFPROPERTY,DEFVAL,VDCLASS) \
const wxPGValueType *wxPGValueType_##VALUETYPE = (wxPGValueType*)NULL; \
class wxPGValueType##VALUETYPE##Class : public wxPGValueType \
{ \
protected: \
    VALUETYPE   m_default; \
public: \
    virtual wxPG_CONST_WXCHAR_PTR GetTypeName() const { return wxT(#VALUETYPE); } \
    virtual wxPG_CONST_WXCHAR_PTR GetCustomTypeName() const { return wxT(#VALUETYPE); } \
    virtual wxPGVariant GetDefaultValue() const { return wxPGVariantCreator((void*)&m_default); } \
    virtual wxPGProperty* GenerateProperty( const wxString& label, const wxString& name ) const \
    { \
        return wxPG_CONSTFUNC(DEFPROPERTY)(label,name); \
    } \
    virtual void SetValueFromVariant( wxPGProperty* property, wxVariant& value ) const \
    { \
        wxPG_CHECK_RET_DBG( wxStrcmp(GetTypeName(),value.GetType().c_str()) == 0, \
            wxT("SetValueFromVariant: wxVariant type mismatch.") ); \
        VDCLASS* vd = (VDCLASS*)value.GetData(); \
        wxPG_CHECK_RET_DBG( wxDynamicCastVariantData(vd, VDCLASS), \
            wxT("SetValueFromVariant: wxVariantData mismatch.")); \
        property->DoSetValue((void*)&vd->GetValue() ); \
    } \
    wxPGValueType##VALUETYPE##Class() { m_default = DEFVAL; } \
    virtual ~wxPGValueType##VALUETYPE##Class() { }


#define WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_SIMPLE(VALUETYPE,DEFPROPERTY,DEFVAL) \
WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_BASE(VALUETYPE,DEFPROPERTY,DEFVAL,wxVariantData_##VALUETYPE) \
WX_PG_GENVARIANT_VOIDP_SIMPLE() \
}; \
WX_PG_IMPLEMENT_VALUE_TYPE_CREATOR(VALUETYPE)


// If you use this macro in application code, you need to pair it with
// WX_PG_DECLARE_VALUE_TYPE with this instead of _VOIDP version.
#define WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_CVD(VALUETYPE,DEFPROPERTY,DEFVAL,VDCLASS) \
WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_BASE(VALUETYPE,DEFPROPERTY,DEFVAL,VDCLASS) \
WX_PG_GENVARIANT_VOIDP_CVD(VDCLASS, VALUETYPE) \
}; \
WX_PG_IMPLEMENT_VALUE_TYPE_CREATOR(VALUETYPE)

#define WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP2(VALUETYPE,DEFPROPERTY,DEFVAL,VDCLASS) \
WX_PG_IMPLEMENT_VALUE_TYPE_VDC(VDCLASS,VALUETYPE) \
WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_CVD(VALUETYPE,DEFPROPERTY,DEFVAL,VDCLASS)

#define WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP(VALUETYPE,DEFPROPERTY,DEFVAL) \
WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP2(VALUETYPE,DEFPROPERTY,DEFVAL,wxVariantData_##VALUETYPE)

//
// Use this to create a new type with a different default value.
// NOTE: With this type you need to use wxPG_INIT_REQUIRED_TYPE2
//   instead of wxPG_INIT_REQUIRED_TYPE.
#define WX_PG_IMPLEMENT_DERIVED_TYPE(VALUETYPE,PARENTVT,DEFVAL) \
const wxPGValueType *wxPGValueType_##VALUETYPE = (wxPGValueType*) NULL; \
class wxPGValueType##VALUETYPE##Class : public wxPGValueType \
{ \
protected: \
    const wxPGValueType*    m_parentClass; \
    PARENTVT                m_default; \
public: \
    virtual wxPG_CONST_WXCHAR_PTR GetTypeName() const { return m_parentClass->GetTypeName(); } \
    virtual wxPG_CONST_WXCHAR_PTR GetCustomTypeName() const { return wxT(#VALUETYPE); } \
    virtual wxPGVariant GetDefaultValue() const { return wxPGVariantCreator(m_default); } \
    virtual wxVariant GenerateVariant ( wxPGVariant value, const wxString& name ) const \
    { return m_parentClass->GenerateVariant(value,name); } \
    virtual wxPGProperty* GenerateProperty ( const wxString& label, const wxString& name ) const \
    { return m_parentClass->GenerateProperty(label,name); } \
    virtual void SetValueFromVariant ( wxPGProperty* property, wxVariant& value ) const \
    { m_parentClass->SetValueFromVariant(property,value); } \
    wxPGValueType##VALUETYPE##Class(); \
    virtual ~wxPGValueType##VALUETYPE##Class(); \
}; \
wxPGValueType##VALUETYPE##Class::wxPGValueType##VALUETYPE##Class() \
{ \
    m_default = DEFVAL; \
    m_parentClass = wxPGValueType_##PARENTVT; \
    wxASSERT( m_parentClass != (wxPGValueType*) NULL); \
} \
wxPGValueType##VALUETYPE##Class::~wxPGValueType##VALUETYPE##Class() { }

// -----------------------------------------------------------------------

//
// Additional property class declaration helper macros
//

#define WX_PG_DECLARE_DERIVED_PROPERTY_CLASS() \
public: \
    WX_PG_DECLARE_GETCLASSNAME \
    WX_PG_DECLARE_GETCLASSINFO \
private:

// -----------------------------------------------------------------------

//
// Property class implementation helper macros.
//

#define WX_PG_DECLARE_PARENTAL_TYPE_METHODS() \
    virtual void DoSetValue ( wxPGVariant value ); \
    virtual wxPGVariant DoGetValue () const;

#define WX_PG_DECLARE_BASIC_TYPE_METHODS() \
    virtual void DoSetValue ( wxPGVariant value ); \
    virtual wxPGVariant DoGetValue () const; \
    virtual wxString GetValueAsString ( int argFlags = 0 ) const; \
    virtual bool SetValueFromString ( const wxString& text, int flags = 0 );

// class WXDLLIMPEXP_PG
#define wxPG_BEGIN_PROPERTY_CLASS_BODY2(CLASSNAME,UPCLASS,T,INTERNAL_T,T_AS_ARG,DECL) \
DECL CLASSNAME : public UPCLASS \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS_NOPARENS \
protected: \
    INTERNAL_T      m_value; \
public: \
    CLASSNAME( const wxString& label, const wxString& name, T_AS_ARG value ); \
    virtual ~CLASSNAME();

#define wxPG_BEGIN_PROPERTY_CLASS_BODY(NAME,UPCLASS,T,T_AS_ARG) \
wxPG_BEGIN_PROPERTY_CLASS_BODY2(wxPG_PROPCLASS(NAME),UPCLASS,T,T,T_AS_ARG,class)

#define wxPG_BEGIN_PROPERTY_CLASS_BODY_WITH_DECL(NAME,UPCLASS,T,T_AS_ARG,DECL) \
wxPG_BEGIN_PROPERTY_CLASS_BODY2(wxPG_PROPCLASS(NAME),UPCLASS,T,T,T_AS_ARG,class DECL)

#define wxPG_END_PROPERTY_CLASS_BODY() \
};

#define WX_PG_DECLARE_CHOICE_METHODS() \
    virtual bool SetValueFromInt( long value, int flags = 0 ); \
    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

#define WX_PG_DECLARE_EVENT_METHODS() \
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event );

#define WX_PG_DECLARE_PARENTAL_METHODS() \
    virtual void ChildChanged( wxPGProperty* p ); \
    virtual void RefreshChildren();

#define WX_PG_DECLARE_CUSTOM_PAINT_METHODS() \
    virtual wxSize GetImageSize() const; \
    virtual void OnCustomPaint( wxDC& dc, const wxRect& rect, wxPGPaintData& paintdata );

#define WX_PG_DECLARE_ATTRIBUTE_METHODS() \
    virtual void SetAttribute( int id, wxVariant& value );

#if wxUSE_VALIDATORS
    #define WX_PG_DECLARE_VALIDATOR_METHODS() \
        virtual wxValidator* DoGetValidator() const;
#else
    #define WX_PG_DECLARE_VALIDATOR_METHODS()
#endif

#if wxPG_INCLUDE_WXOBJECT
    #define wxPG_GETCLASSNAME_IMPLEMENTATION(PROPNAME)
#else
    #define wxPG_GETCLASSNAME_IMPLEMENTATION(PROPNAME) \
    static const wxChar* PROPNAME##_ClassName = wxT(#PROPNAME); \
    wxPG_CONST_WXCHAR_PTR PROPNAME##Class::GetClassName() const { return PROPNAME##_ClassName; }
#endif

// Implements sans constructor function. Also, first arg is class name, not property name.
#define WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN(PROPNAME,T,EDITOR) \
wxPG_GETCLASSNAME_IMPLEMENTATION(PROPNAME) \
wxPG_VALUETYPE_MSGVAL PROPNAME##Class::GetValueType() const \
{ \
    return wxPG_VALUETYPE(T); \
} \
const wxPGEditor* PROPNAME##Class::DoGetEditorClass() const \
{ \
    return wxPGEditor_##EDITOR; \
}


#define WX_PG_IMPLEMENT_CLASSINFO(NAME,UPCLASS) \
static wxPGProperty* NAME##Class2(const wxString& label, const wxString& name) \
{ \
    return wxPG_CONSTFUNC(NAME)(label,name); \
} \
wxPGPropertyClassInfo NAME##ClassInfo = {wxT(#NAME),&UPCLASS##Info,&NAME##Class2}; \
const wxPGPropertyClassInfo* NAME##Class::GetClassInfo() const \
{ \
    return &NAME##ClassInfo; \
}


// second constfunc and classinfo is for creating properties w/o value from text
// (should be changed to simpler scheme in 1.2)
#define WX_PG_IMPLEMENT_CONSTFUNC(NAME,T_AS_ARG) \
wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name, T_AS_ARG value ) \
{ \
    return new wxPG_PROPCLASS(NAME)(label,name,value); \
}

// Adds constructor function as well.
#define WX_PG_IMPLEMENT_PROPERTY_CLASS2(NAME,CLASSNAME,UPCLASS,T,T_AS_ARG,EDITOR) \
WX_PG_IMPLEMENT_CONSTFUNC(NAME,T_AS_ARG) \
WX_PG_IMPLEMENT_CLASSINFO(NAME,UPCLASS) \
WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN(NAME,T,EDITOR)

// A regular property
#define WX_PG_IMPLEMENT_PROPERTY_CLASS(NAME,UPNAME,T,T_AS_ARG,EDITOR) \
WX_PG_IMPLEMENT_PROPERTY_CLASS2(NAME,wxPG_PROPCLASS(NAME),wxPG_PROPCLASS(UPNAME),T,T_AS_ARG,EDITOR)

// Derived property class is one that inherits from an existing working property
// class, but assumes same value and editor type.
#define WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(NAME,UPNAME,T_AS_ARG) \
WX_PG_IMPLEMENT_CONSTFUNC(NAME,T_AS_ARG) \
WX_PG_IMPLEMENT_CLASSINFO(NAME,UPNAME##Class) \
wxPG_GETCLASSNAME_IMPLEMENTATION(NAME)

// -----------------------------------------------------------------------

#define wxPG_NO_ESCAPE      wxPG_PROP_NO_ESCAPE     // No escape sequences
#define wxPG_ESCAPE         0                       // Escape sequences

#define WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR(NAME,FLAGS) \
class NAME##Class : public wxLongStringPropertyClass \
{ \
    WX_PG_DECLARE_DERIVED_PROPERTY_CLASS() \
public: \
    NAME##Class( const wxString& name, const wxString& label, const wxString& value ); \
    virtual ~NAME##Class(); \
    virtual bool OnButtonClick( wxPropertyGrid* propgrid, wxString& value ); \
    WX_PG_DECLARE_VALIDATOR_METHODS() \
}; \
WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(NAME,wxLongStringProperty,const wxString&) \
NAME##Class::NAME##Class( const wxString& name, const wxString& label, const wxString& value ) \
  : wxLongStringPropertyClass(name,label,value) \
{ \
    m_flags |= FLAGS; \
} \
NAME##Class::~NAME##Class() { }

#if wxUSE_VALIDATORS

#define WX_PG_IMPLEMENT_STRING_PROPERTY(NAME,FLAGS) \
WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR(NAME,FLAGS) \
wxValidator* wxPG_PROPCLASS(NAME)::DoGetValidator () const \
{ return (wxValidator*) NULL; }

#else

#define WX_PG_IMPLEMENT_STRING_PROPERTY WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR

#endif

// -----------------------------------------------------------------------

// This will create interface for wxFlagsProperty derived class
// named CLASSNAME.
#define WX_PG_IMPLEMENT_CUSTOM_FLAGS_PROPERTY2(NAME,CLASSNAME,LABELS,VALUES,DEFVAL) \
class CLASSNAME : public wxFlagsPropertyClass \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS() \
public: \
    CLASSNAME( const wxString& label, const wxString& name, long value ); \
    virtual ~CLASSNAME(); \
}; \
WX_PG_IMPLEMENT_DERIVED_TYPE(long_##NAME,long,DEFVAL) \
WX_PG_IMPLEMENT_PROPERTY_CLASS(NAME,wxFlagsProperty,long_##NAME,long,TextCtrl) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, long value ) \
    : wxFlagsPropertyClass(label,name,LABELS,VALUES,value!=-1?value:DEFVAL) \
{ \
    wxPG_INIT_REQUIRED_TYPE2(long_##NAME) \
    m_flags |= wxPG_PROP_STATIC_CHOICES; \
} \
CLASSNAME::~CLASSNAME() { }

#define WX_PG_IMPLEMENT_CUSTOM_FLAGS_PROPERTY(NAME,LABELS,VALUES,DEFVAL) \
WX_PG_IMPLEMENT_CUSTOM_FLAGS_PROPERTY2(NAME,wxPG_PROPCLASS(NAME),LABELS,VALUES,DEFVAL)

// -----------------------------------------------------------------------

// This will create interface for Enum property derived class
// named CLASSNAME.
#define WX_PG_IMPLEMENT_CUSTOM_ENUM_PROPERTY2(NAME,CLASSNAME,LABELS,VALUES,DEFVAL) \
class CLASSNAME : public wxEnumPropertyClass \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS() \
public: \
    CLASSNAME ( const wxString& label, const wxString& name, int value ); \
    virtual ~CLASSNAME(); \
}; \
WX_PG_IMPLEMENT_DERIVED_TYPE(long_##NAME,long,DEFVAL) \
WX_PG_IMPLEMENT_PROPERTY_CLASS(NAME,wxEnumProperty,long_##NAME,int,Choice) \
CLASSNAME::CLASSNAME ( const wxString& label, const wxString& name, int value ) \
    : wxEnumPropertyClass(label,name,LABELS,VALUES,value!=-1?value:DEFVAL) \
{ \
    wxPG_INIT_REQUIRED_TYPE2(long_##NAME) \
    m_flags |= wxPG_PROP_STATIC_CHOICES; \
} \
CLASSNAME::~CLASSNAME() { }

#define WX_PG_IMPLEMENT_CUSTOM_ENUM_PROPERTY(NAME,LABELS,VALUES,DEFVAL) \
WX_PG_IMPLEMENT_CUSTOM_ENUM_PROPERTY2(NAME,wxPG_PROPCLASS(NAME),LABELS,VALUES,DEFVAL)

// -----------------------------------------------------------------------

// Implementation for user wxColour editor property

#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY2(NAME,CLASSNAME,LABELS,VALUES,COLOURS) \
class CLASSNAME : public wxPG_PROPCLASS(wxSystemColourProperty) \
{ \
    WX_PG_DECLARE_DERIVED_PROPERTY_CLASS() \
public: \
    CLASSNAME( const wxString& label, const wxString& name, \
        const wxColourPropertyValue& value ); \
    virtual ~CLASSNAME (); \
    virtual long GetColour ( int index ); \
}; \
static wxPGChoices gs_##NAME##_choicesCache; \
WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(NAME,wxSystemColourProperty,const wxColourPropertyValue&) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, \
    const wxColourPropertyValue& value ) \
    : wxPG_PROPCLASS(wxSystemColourProperty)(label,name,LABELS,VALUES,&gs_##NAME##_choicesCache,value ) \
{ \
    wxPG_INIT_REQUIRED_TYPE(wxColourPropertyValue) \
    m_flags |= wxPG_PROP_TRANSLATE_CUSTOM; \
    DoSetValue ( &m_value ); \
} \
CLASSNAME::~CLASSNAME () { } \
long CLASSNAME::GetColour ( int index ) \
{ \
    const wxArrayInt& values = GetValues(); \
    if ( !values.GetCount() ) \
    { \
        wxASSERT( index < (int)m_choices.GetCount() ); \
        return COLOURS[index]; \
    } \
    return COLOURS[values[index]]; \
}

#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY(NAME,LABELS,VALUES,COLOURS) \
    WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY2(NAME,wxPG_PROPCLASS(NAME),LABELS,VALUES,COLOURS)

// -----------------------------------------------------------------------

#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR2(NAME,CLASSNAME,LABELS,VALUES,COLOURS,EDITOR) \
class CLASSNAME : public wxPG_PROPCLASS(wxSystemColourProperty) \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS() \
public: \
    CLASSNAME( const wxString& label, const wxString& name, \
        const wxColour& value ); \
    virtual ~CLASSNAME (); \
    virtual void DoSetValue ( wxPGVariant value ); \
    virtual wxPGVariant DoGetValue() const; \
    virtual wxString GetValueAsString ( int argFlags ) const; \
    virtual long GetColour ( int index ); \
}; \
static wxPGChoices gs_##NAME##_choicesCache; \
WX_PG_IMPLEMENT_PROPERTY_CLASS(NAME,wxSystemColourProperty,wxColour,const wxColour&,EDITOR) \
CLASSNAME::CLASSNAME( const wxString& label, const wxString& name, const wxColour& value ) \
    : wxPG_PROPCLASS(wxSystemColourProperty)(label,name,LABELS,VALUES,&gs_##NAME##_choicesCache,value ) \
{ \
    wxPG_INIT_REQUIRED_TYPE(wxColour) \
    m_flags |= wxPG_PROP_TRANSLATE_CUSTOM; \
    DoSetValue(&m_value.m_colour); \
} \
CLASSNAME::~CLASSNAME () { } \
void CLASSNAME::DoSetValue( wxPGVariant value ) \
{ \
    wxColour* pval = wxPGVariantToWxObjectPtr(value, wxColour); \
    wxCHECK_RET(pval, wxT("NULL wxColour pointer")); \
    m_value.m_type = wxPG_COLOUR_CUSTOM; \
    if ( m_flags & wxPG_PROP_TRANSLATE_CUSTOM ) \
    { \
        int found_ind = ColToInd(*pval); \
        if ( found_ind != wxNOT_FOUND ) m_value.m_type = found_ind; \
    } \
    m_value.m_colour = *pval; \
    if ( m_value.m_type < wxPG_COLOUR_WEB_BASE ) \
        wxPG_PROPCLASS(wxEnumProperty)::DoSetValue ( (long)m_value.m_type ); \
    else \
        m_index = GetItemCount()-1; \
} \
wxPGVariant CLASSNAME::DoGetValue() const \
{ \
    return wxPGVariantCreator(m_value.m_colour); \
} \
wxString CLASSNAME::GetValueAsString( int argFlags ) const \
{ \
    const wxPGEditor* editor = GetEditorClass(); \
    if ( editor != wxPG_EDITOR(Choice) && \
         editor != wxPG_EDITOR(ChoiceAndButton) ) \
        argFlags |= wxPG_PROPERTY_SPECIFIC; \
    return wxSystemColourPropertyClass::GetValueAsString(argFlags); \
} \
long CLASSNAME::GetColour( int index ) \
{ \
    const wxArrayInt& values = GetValues(); \
    if ( !values.GetCount() ) \
    { \
        wxASSERT( index < (int)GetItemCount() ); \
        return COLOURS[index]; \
    } \
    return COLOURS[values[index]]; \
}

#define WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(NAME,LABELS,VALUES,COLOURS) \
    WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR2(NAME,wxPG_PROPCLASS(NAME),LABELS,VALUES,COLOURS,Choice)

// -----------------------------------------------------------------------

//
// These macros helps creating DoGetValidator
#define WX_PG_DOGETVALIDATOR_ENTRY() \
    WX_PG_GLOBALS_LOCKER() \
    static wxValidator* s_ptr = (wxValidator*) NULL; \
    if ( s_ptr ) return s_ptr;

// Common function exit
#define WX_PG_DOGETVALIDATOR_EXIT(VALIDATOR) \
    s_ptr = VALIDATOR; \
    wxPGGlobalVars->m_arrValidators.Add( (void*) VALIDATOR ); \
    return VALIDATOR;

// -----------------------------------------------------------------------

//
// Ids for sub-controls
// NB: It should not matter what these are.
#define wxPG_SUBID1                     11485
#define wxPG_SUBID2                     11486
#define wxPG_SUBID_TEMP1                11487

// -----------------------------------------------------------------------

/** \class wxPGPaintData
    \ingroup classes
    \brief Contains information relayed to property's OnCustomPaint.
*/
struct wxPGPaintData
{
    /** wxPropertyGrid. */
    const wxPropertyGrid*   m_parent;

    /** Normally -1, otherwise index to drop-down list item that has to be drawn. */
    int                     m_choiceItem;

    /** Set to drawn width in OnCustomPaint (optional). */
    int                     m_drawnWidth;

    /** In a measure item call, set this to the height of item at m_choiceItem index. */
    int                     m_drawnHeight;

    
};


// -----------------------------------------------------------------------

#ifndef SWIG

/** \class wxPGInDialogValidator
    \ingroup classes
    \brief Creates and manages a temporary wxTextCtrl for validation purposes.
    Uses wxPropertyGrid's current editor, if available.
*/
class WXDLLIMPEXP_PG wxPGInDialogValidator
{
public:
    wxPGInDialogValidator()
    {
        m_textCtrl = NULL;
    }

    ~wxPGInDialogValidator()
    {
        if ( m_textCtrl )
            m_textCtrl->Destroy();
    }

    bool DoValidate( wxPropertyGrid* propGrid, wxValidator* validator, const wxString& value );
    
private:
    wxTextCtrl*         m_textCtrl;
};

#endif


#ifndef DOXYGEN


// -----------------------------------------------------------------------
// Some property class definitions (these should be useful to inherit from).
// -----------------------------------------------------------------------

//#ifndef SWIG
#if 1

#define wxPG_PROP_PASSWORD  wxPG_PROP_CLASS_SPECIFIC_2

class WXDLLIMPEXP_PG wxStringPropertyClass : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS_NOPARENS
public:
    wxStringPropertyClass( const wxString& label, const wxString& name, const wxString& value );
    virtual ~wxStringPropertyClass();

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()

protected:
    wxString        m_value;
};

// -----------------------------------------------------------------------

// This provides base for wxEnumPropertyClass and any custom
// "dynamic" enum property classes.
class WXDLLIMPEXP_PG wxBaseEnumPropertyClass : public wxPGProperty
{
public:
    wxBaseEnumPropertyClass( const wxString& label, const wxString& name );

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual wxString GetValueAsString( int argFlags ) const;
    virtual bool SetValueFromString( const wxString& text, int argFlags );
    virtual bool SetValueFromInt( long value, int argFlags );

    //
    // Additional virtuals

    // This must be overridden to have non-index based value
    virtual int GetIndexForValue( int value ) const;

    // This returns string and value for index
    // Returns NULL if beyond last item
    // pvalue is never NULL - always set it.
    virtual const wxString* GetEntry( size_t index, int* pvalue ) const = 0;

protected:

    int                     m_index;
};

// -----------------------------------------------------------------------

// If set, then selection of choices is static and should not be
// changed (i.e. returns NULL in GetPropertyChoices).
#define wxPG_PROP_STATIC_CHOICES    wxPG_PROP_CLASS_SPECIFIC_1

class WXDLLIMPEXP_PG wxEnumPropertyClass : public wxBaseEnumPropertyClass
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

#ifndef SWIG
    wxEnumPropertyClass( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values = NULL, int value = 0 );
    wxEnumPropertyClass( const wxString& label, const wxString& name, 
        wxPGChoices& choices, int value = 0 );

    // Special constructor for caching choices (used by derived class)
    wxEnumPropertyClass( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values, wxPGChoices* choicesCache, int value = 0 );
#endif
    wxEnumPropertyClass( const wxString& label, const wxString& name,
        const wxArrayString& labels, const wxArrayInt& values = wxArrayInt(), int value = 0 );

    virtual ~wxEnumPropertyClass();

    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );
    virtual int GetIndexForValue( int value ) const;
    virtual const wxString* GetEntry( size_t index, int* pvalue ) const;

    inline size_t GetItemCount() const { return m_choices.GetCount(); }
    inline const wxArrayInt& GetValues() const { return m_choices.GetValues(); }

protected:
    wxPGChoices             m_choices;
};

// -----------------------------------------------------------------------

class WXDLLIMPEXP_PG wxFlagsPropertyClass : public wxPGPropertyWithChildren
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

#ifndef SWIG
    wxFlagsPropertyClass( const wxString& label, const wxString& name, const wxChar** labels,
        const long* values = NULL, long value = 0 );
    wxFlagsPropertyClass( const wxString& label, const wxString& name, 
        wxPGChoices& choices, long value = 0 );
#endif
    wxFlagsPropertyClass( const wxString& label, const wxString& name,
        const wxArrayString& labels, const wxArrayInt& values, int value = 0 );
    virtual ~wxFlagsPropertyClass ();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual wxString GetValueAsString( int argFlags ) const;
    virtual bool SetValueFromString( const wxString& text, int flags );
    virtual void ChildChanged( wxPGProperty* p );
    virtual void RefreshChildren();

    // this is necessary for conveying m_choices
    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

    // helpers
    inline size_t GetItemCount() const { return m_choices.GetCount(); }
    inline const wxArrayInt& GetValues() const { return m_choices.GetValues(); }
    inline const wxString& GetLabel( size_t ind ) const { return m_choices.GetLabel(ind); }

protected:
    wxPGChoices             m_choices;

    // Used to detect if choices have been changed
    wxPGChoicesData*        m_oldChoicesData;

    long                    m_value;

    // Converts string id to a relevant bit.
    long IdToBit( const wxString& id ) const;

    // Creates children and sets value.
    void Init();
};

// -----------------------------------------------------------------------

#include <wx/filename.h>

// Indicates first bit useable by derived properties.
#define wxPG_PROP_SHOW_FULL_FILENAME  wxPG_PROP_CLASS_SPECIFIC_1

class WXDLLIMPEXP_PG wxFilePropertyClass : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxFilePropertyClass( const wxString& label, const wxString& name = wxPG_LABEL,
        const wxString& value = wxEmptyString );
    virtual ~wxFilePropertyClass ();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual wxString GetValueAsString( int argFlags ) const;
    virtual bool SetValueFromString( const wxString& text, int flags );
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* wnd_primary, wxEvent& event );

    virtual void SetAttribute( int id, wxVariant& value );

#if wxUSE_VALIDATORS
    static wxValidator* GetClassValidator();
    virtual wxValidator* DoGetValidator() const;
#endif

protected:
    wxString    m_wildcard;
    wxString    m_fnstr; // needed for return value
    wxString    m_basePath; // If set, then show path relative to it
    wxString    m_initialPath; // If set, start the file dialog here
    wxString    m_dlgTitle; // If set, used as title for file dialog
    wxFileName  m_filename; // used as primary storage
    int         m_indFilter; // index to the selected filter
};

// -----------------------------------------------------------------------

#define wxPG_PROP_NO_ESCAPE     wxPG_PROP_CLASS_SPECIFIC_1

//
// In wxTextCtrl, strings a space delimited C-like strings. For example:
// "String 1" "String 2" "String 3"
//
// To have " in a string, use \".
// To have \ in a string, use \\.
//
class WXDLLIMPEXP_PG wxLongStringPropertyClass : public wxBasePropertyClass
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxLongStringPropertyClass( const wxString& label, const wxString& name = wxPG_LABEL, const wxString& value = wxEmptyString );
    virtual ~wxLongStringPropertyClass();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual wxString GetValueAsString( int argFlags = 0 ) const;
    virtual bool SetValueFromString( const wxString& text, int flags );

    WX_PG_DECLARE_EVENT_METHODS()

    //  Shows string editor dialog. Value to be edited should be read from value, and
    //  if dialog is not cancelled, it should be stored back and true should be returned
    //  if that was the case.
    virtual bool OnButtonClick( wxPropertyGrid* propgrid, wxString& value );

protected:
    wxString    m_value;
};

// -----------------------------------------------------------------------

// wxBoolProperty specific flags
#define wxPG_PROP_USE_CHECKBOX      wxPG_PROP_CLASS_SPECIFIC_1
// DCC = Double Click Cycles
#define wxPG_PROP_USE_DCC           wxPG_PROP_CLASS_SPECIFIC_2


// -----------------------------------------------------------------------

class WXDLLIMPEXP_PG wxArrayStringPropertyClass : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxArrayStringPropertyClass( const wxString& label,
                                const wxString& name,
                                const wxArrayString& value );
    virtual ~wxArrayStringPropertyClass();

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_EVENT_METHODS()

    virtual void GenerateValueAsString();

    //  Shows string editor dialog. Value to be edited should be read from value, and
    //  if dialog is not cancelled, it should be stored back and true should be returned
    //  if that was the case.
    virtual bool OnCustomStringEdit( wxWindow* parent, wxString& value );

    // Helper.
    virtual bool OnButtonClick( wxPropertyGrid* propgrid,
                                wxWindow* primary,
                                const wxChar* cbt );

    // Creates wxArrayEditorDialog for string editing. Called in OnButtonClick.
    virtual wxArrayEditorDialog* CreateEditorDialog();

protected:
    wxArrayString   m_value;
    wxString        m_display; // Cache for displayed text.
};

#define WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAME,DELIMCHAR,CUSTBUTTXT) \
class wxPG_PROPCLASS(PROPNAME) : public wxPG_PROPCLASS(wxArrayStringProperty) \
{ \
    WX_PG_DECLARE_PROPERTY_CLASS() \
public: \
    wxPG_PROPCLASS(PROPNAME)( const wxString& label, const wxString& name, const wxArrayString& value ); \
    ~wxPG_PROPCLASS(PROPNAME)(); \
    virtual void GenerateValueAsString(); \
    virtual bool SetValueFromString( const wxString& text, int ); \
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event ); \
    virtual bool OnCustomStringEdit( wxWindow* parent, wxString& value ); \
    WX_PG_DECLARE_VALIDATOR_METHODS() \
}; \
WX_PG_IMPLEMENT_PROPERTY_CLASS(PROPNAME,wxArrayStringProperty,wxArrayString,const wxArrayString&,TextCtrlAndButton) \
wxPG_PROPCLASS(PROPNAME)::wxPG_PROPCLASS(PROPNAME) ( const wxString& label, const wxString& name, const wxArrayString& value ) \
    : wxPG_PROPCLASS(wxArrayStringProperty)(label,name,value) \
{ \
    wxPG_PROPCLASS(PROPNAME)::GenerateValueAsString(); \
} \
wxPG_PROPCLASS(PROPNAME)::~wxPG_PROPCLASS(PROPNAME)() { } \
void wxPG_PROPCLASS(PROPNAME)::GenerateValueAsString() \
{ \
    wxChar delimChar = DELIMCHAR; \
    if ( delimChar == wxT('"') ) \
        wxPG_PROPCLASS(wxArrayStringProperty)::GenerateValueAsString(); \
    else \
        wxPropertyGrid::ArrayStringToString(m_display,m_value,0,DELIMCHAR,0); \
} \
bool wxPG_PROPCLASS(PROPNAME)::SetValueFromString( const wxString& text, int ) \
{ \
    wxChar delimChar = DELIMCHAR; \
    if ( delimChar == wxT('"') ) \
        return wxPG_PROPCLASS(wxArrayStringProperty)::SetValueFromString(text,0); \
    \
    m_value.Empty(); \
    WX_PG_TOKENIZER1_BEGIN(text,DELIMCHAR) \
        m_value.Add( token ); \
    WX_PG_TOKENIZER1_END() \
    GenerateValueAsString(); \
    return true; \
} \
bool wxPG_PROPCLASS(PROPNAME)::OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event ) \
{ \
    if ( event.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED ) \
        return OnButtonClick(propgrid,primary,(const wxChar*) CUSTBUTTXT); \
    return false; \
}

#if wxUSE_VALIDATORS

#define WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY(PROPNAME,DELIMCHAR,CUSTBUTTXT) \
WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY_WITH_VALIDATOR(PROPNAME,DELIMCHAR,CUSTBUTTXT) \
wxValidator* wxPG_PROPCLASS(PROPNAME)::DoGetValidator () const \
{ return (wxValidator*) NULL; }

#else

#define WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY_WITH_VALIDATOR WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY

#endif


// -----------------------------------------------------------------------
// wxPGEditorDialog
//   TODO: To be enabled for 1.3.
// -----------------------------------------------------------------------

#if 0
class wxPGEditorDialog : public wxDialog
{
public:

    wxPGEditorDialog() : wxDialog()
    {
        m_modified = false;
    }

    /** Called instead non-virtual Create. Must call wxDialog::Create internally.
        Note that wxPropertyGrid is always dialog's parent.

        \params
        custBtText: Allow setting single custom button action. Text is
          button title. Event must be intercepted in property's OnEvent()
          member function. Not all dialogs are expected to support this.
    */
    virtual bool VCreate( wxPropertyGrid* pg,
                          wxPGProperty* p,
                          const wxString& caption,
                          const wxString& message,
                          wxVariant value,
                          const wxChar* custBtText = NULL ) = 0;

    virtual wxVariant GetValue() const = 0;

    // Returns true if value was actually modified
    inline bool IsModified() const { return m_modified; }

protected:

    bool        m_modified;

private:
};
#endif

// -----------------------------------------------------------------------
// wxArrayEditorDialog
// -----------------------------------------------------------------------

#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/listbox.h>

#define wxAEDIALOG_STYLE \
    (wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxOK | wxCANCEL | wxCENTRE)

class WXDLLIMPEXP_PG wxArrayEditorDialog : public wxDialog
{
public:
    wxArrayEditorDialog();

    void Init();

    wxArrayEditorDialog( wxWindow *parent,
                         const wxString& message,
                         const wxString& caption,
                         long style = wxAEDIALOG_STYLE,
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& sz = wxDefaultSize );

    bool Create( wxWindow *parent,
                 const wxString& message,
                 const wxString& caption,
                 long style = wxAEDIALOG_STYLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& sz = wxDefaultSize );

    /** Set value modified by dialog.
    */
    virtual void SetDialogValue( const wxVariant& WXUNUSED(value) )
    {
        wxFAIL_MSG(wxT("re-implement this member function in derived class"));
    }

    /** Return value modified by dialog.
    */
    virtual wxVariant GetDialogValue() const
    {
        wxFAIL_MSG(wxT("re-implement this member function in derived class"));
        return wxVariant();
    }

    /** Override to return wxValidator to be used with the wxTextCtrl
        in dialog. Note that the validator is used in the standard 
        wx way, ie. it immediately prevents user from entering invalid
        input.

        \remarks
        Dialog frees the validator.
    */
    virtual wxValidator* GetTextCtrlValidator() const
    {
        return (wxValidator*) NULL;
    }

    // Returns true if array was actually modified
    bool IsModified() const { return m_modified; }

    //const wxArrayString& GetStrings() const { return m_array; }

    // implementation from now on
    void OnUpdateClick(wxCommandEvent& event);
    void OnAddClick(wxCommandEvent& event);
    void OnDeleteClick(wxCommandEvent& event);
    void OnListBoxClick(wxCommandEvent& event);
    void OnUpClick(wxCommandEvent& event);
    void OnDownClick(wxCommandEvent& event);
    //void OnCustomEditClick(wxCommandEvent& event);
    void OnIdle(wxIdleEvent& event);

protected:
    wxTextCtrl*     m_edValue;
    wxListBox*      m_lbStrings;

    wxButton*       m_butAdd;       // Button pointers
    wxButton*       m_butCustom;    // required for disabling/enabling changing.
    wxButton*       m_butUpdate;
    wxButton*       m_butRemove;
    wxButton*       m_butUp;
    wxButton*       m_butDown;

    //wxArrayString   m_array;

    const wxChar*   m_custBtText;
    //wxArrayStringPropertyClass*     m_pCallingClass;

    bool            m_modified;

    unsigned char   m_curFocus;

    // These must be overridden - must return true on success.
    virtual wxString ArrayGet( size_t index ) = 0;
    virtual size_t ArrayGetCount() = 0;
    virtual bool ArrayInsert( const wxString& str, int index ) = 0;
    virtual bool ArraySet( size_t index, const wxString& str ) = 0;
    virtual void ArrayRemoveAt( int index ) = 0;
    virtual void ArraySwap( size_t first, size_t second ) = 0;

private:
#ifndef SWIG
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxArrayEditorDialog)
    DECLARE_EVENT_TABLE()
#endif
};

// -----------------------------------------------------------------------


/** This is a simple property which holds sub-properties. Has default editing
    textctrl based editing capability. In essence, it is a category that has
    look and feel of a property, and which children can be edited via the textctrl.
*/
class WXDLLIMPEXP_PG wxParentPropertyClass : public wxPGPropertyWithChildren
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxParentPropertyClass( const wxString& label, const wxString& name = wxPG_LABEL );
    virtual ~wxParentPropertyClass();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual void ChildChanged( wxPGProperty* p );
    virtual wxString GetValueAsString( int argFlags = 0 ) const;

protected:
    wxString    m_string;
};


// -----------------------------------------------------------------------

#endif // #ifndef SWIG

/** \class wxCustomPropertyClass
    \ingroup classes
    \brief This is a rather inefficient but very versatile property class.

   Base class offers the following:
     - Add any properties as children (i.e. like wxParentProperty)
     - Editor control can be set at run-time.
     - By default has string value type.
     - Has capacity to have choices.
     - Can have custom-paint bitmap.

   Also note:
     - Has m_parentingType of -2 (technical detail).
*/
class WXDLLIMPEXP_PG wxCustomPropertyClass : public wxPGPropertyWithChildren
{
#ifndef SWIG
    WX_PG_DECLARE_PROPERTY_CLASS()
#endif
public:

    wxCustomPropertyClass( const wxString& label, const wxString& name = wxPG_LABEL );
    virtual ~wxCustomPropertyClass();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual bool SetValueFromString( const wxString& text, int flags );
    virtual wxString GetValueAsString( int argFlags ) const;

#ifdef wxPG_COMPATIBILITY_1_0_0
    virtual bool OnEvent ( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event );
#endif
    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()

    virtual bool SetValueFromInt ( long value, int arg_flags );
    virtual int GetChoiceInfo ( wxPGChoiceInfo* choiceinfo );

    virtual void SetAttribute ( int id, wxVariant& value );

protected:
    //wxPGChoicesData*        m_choices;
    wxPGChoices             m_choices;
#ifdef wxPG_COMPATIBILITY_1_0_0
    wxPropertyGridCallback  m_callback;
#endif
    wxPGPaintCallback       m_paintCallback;

    wxString                m_value;
};

// -----------------------------------------------------------------------

#ifndef SWIG

//
// Tokenizer macros.
// NOTE: I have made two versions - worse ones (performance and consistency
//   wise) use wxStringTokenizer and better ones (may have unfound bugs)
//   use custom code.
//

#include <wx/tokenzr.h>

// TOKENIZER1 can be done with wxStringTokenizer
#define WX_PG_TOKENIZER1_BEGIN(WXSTRING,DELIMITER) \
    wxStringTokenizer tkz(WXSTRING,DELIMITER,wxTOKEN_RET_EMPTY); \
    while ( tkz.HasMoreTokens() ) \
    { \
        wxString token = tkz.GetNextToken(); \
        token.Trim(true); \
        token.Trim(false);

#define WX_PG_TOKENIZER1_END() \
    }


//
// 2nd version: tokens are surrounded by DELIMITERs (for example, C-style strings).
// TOKENIZER2 must use custom code (a class) for full compliancy
// with " surrounded strings with \" inside.
//
// class implementation is in propgrid.cpp
//

class WXDLLIMPEXP_PG wxPGStringTokenizer
{
public:
    wxPGStringTokenizer( const wxString& str, wxChar delimeter );
    ~wxPGStringTokenizer();

    bool HasMoreTokens(); // not const so we can do some stuff in it
    wxString GetNextToken();

protected:
#ifndef SWIG

    const wxString*             m_str;
    wxString::const_iterator    m_curPos;
    wxString                    m_readyToken;
    wxUniChar                   m_delimeter;
#endif
};

#define WX_PG_TOKENIZER2_BEGIN(WXSTRING,DELIMITER) \
    wxPGStringTokenizer tkz(WXSTRING,DELIMITER); \
    while ( tkz.HasMoreTokens() ) \
    { \
        wxString token = tkz.GetNextToken();

#define WX_PG_TOKENIZER2_END() \
    }

#endif

// -----------------------------------------------------------------------

#endif // !DOXYGEN

#endif // _WX_PROPGRID_PROPDEV_H_
