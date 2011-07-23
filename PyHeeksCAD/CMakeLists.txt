project( HeeksCAD-app )
cmake_minimum_required( VERSION 2.6 )

if( NOT DEFINED BUILD_STARTED_IN_RIGHT_DIR )
  message( FATAL_ERROR "Don't start a build in the src directory. Do a 'cd ..', then run cmake ." )
endif( NOT DEFINED BUILD_STARTED_IN_RIGHT_DIR )

set ( CMAKE_BUILD_TYPE Debug )
add_definitions ( -Wall -DUSE_GENERIC_TREECTRL -DHEEKSCAD -DUNICODE -DTIXML_USE_STL )

#find OCE or OpenCASCADE
set( CASCADE_LIBS TKVRML TKSTL TKBRep TKIGES TKShHealing TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXSBase TKShapeSchema FWOSPlugin TKBool TKCAF TKCDF TKernel TKFeat TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKHLR TKMath TKOffset TKPrim TKPShape TKService TKTopAlgo TKV2d TKV3d TKMesh TKAdvTools TKBO TKXDESTEP TKXCAF TKXCAFSchema TKLCAF TKPLCAF )
#inherits variables from parent dir - don't need to 'find_package ( OCE )' again
if( ${OCE_FOUND} )
  set( OpenCASCADE_LIBRARIES ${CASCADE_LIBS} )
  set( OpenCASCADE_INCLUDE_DIR ${OCE_INCLUDE_DIRS} )
else() #look for OpenCASCADE
  set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
  find_package ( OpenCASCADE REQUIRED COMPONENTS ${CASCADE_LIBS} )
endif()

find_package( OpenGL REQUIRED )
FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl aui )
find_package( PythonLibs REQUIRED )
INCLUDE(${wxWidgets_USE_FILE})


include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${wxWidgets_INCLUDE_DIRS} ${OpenCASCADE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS} . ${OPENGL_INCLUDE_DIR}
    )
set ( EXECUTABLE_OUTPUT_PATH ../bin )

set( heekscad_HDRS
        AboutBox.h
        advprops.h
        AutoSave.h
        BentleyOttmann.h
        BezierCurve.h
        Cone.h
        ConstrainedObject.h
        Constraint.h
        ConstraintTools.h
        ConversionTools.h
        CoordinateSystem.h
        CorrelationTool.h
        Cuboid.h
        CxfFont.h
        Cylinder.h
        DigitizedPoint.h
        DigitizeMode.h
        DimensionDrawing.h
        DoubleInput.h
        Drawing.h
        dxf.h
        DynamicSolid.h
        EdgeBlend.h
        Edge.h
        EndedObject.h
        Face.h
        FaceTools.h
        FastCurves.h
        Geom.h
        glfont2.h
        GLList.h
        GraphicsCanvas.h
        Grid.h
        Gripper.h
        GripperSelTransform.h
        Group.h
        HAngularDimension.h
        HArc.h
        HCircle.h
        HDialogs.h
        HDimension.h
        HDxf.h
        HeeksCAD.h
        HeeksCADInterface.h
        HeeksColor.h
        HeeksConfig.h
        HeeksFrame.h
        HeeksObj.h
        HeeksPrintout.h
        HEllipse.h
        HGear.h
        HILine.h
        HImage.h
        HLine.h
        Hole.h
        HPoint.h
        HSpline.h
        HText.h
        HXml.h
# (no Input.h here)
        InputMode.h
        InputModeCanvas.h
        Intersector.h
        LeftAndRight.h
        LineArcDrawing.h
        Loop.h
        MagDragWindow.h
        manager.h
        MarkedList.h
        MarkedObject.h
        material.h
# (there's no matrix.h but here it is??)
        matrix.h
# (there's no MatrixFactorization.h)
        MultiPoly.h
        NearMap.h
        NiceTextCtrl.h
        ObjectCanvas.h
        ObjList.h
        ObjPropsCanvas.h
        Observer.h
        OCCSolver.h
        odcombo.h
        OptionsCanvas.h
        OrientationModifier.h
        Pad.h
        Part.h
        PictureFrame.h
        Plugins.h
        Pocket.h
        PointDrawing.h
        PointOrWindow.h
# (there's no Polygon.h but here it is??)
        Polygon.h
        propdev.h
        PropertiesCanvas.h
        Property.h
        PropertyCheck.h
        PropertyChoice.h
        PropertyColor.h
        PropertyDouble.h
        PropertyInt.h
        PropertyLength.h
        PropertyList.h
        PropertyString.h
        PropertyTrsf.h
        PropertyVertex.h
        propgrid.h
# (no props.h here)
        PythonInterface.h
        RegularShapesDrawing.h
        RS274X.h
        RuledSurface.h
        Ruler.h
        Sectioning.h
        SelectMode.h
        Shape.h
        ShapeData.h
        ShapeTools.h
        SimpleIntersector.h
        Sketch.h
# (no sketchinterface.h here)
        SketchTools.h
        Solid.h
        SolidTools.h
        solve.h
# (no solveimpl.h here)
        SolveSketch.h
        Sphere.h
        stdafx.h
        StlSolid.h
        strconv.h
        StretchTool.h
        svg.h
        timer.h
        tinystr.h
        tinyxml.h
# (no tinyxmlerror.h here)
# (no tinyxmlparser.h here)
        ToolImage.h
        ToolList.h
        TransformTool.h
        TransformTools.h
        TransientObject.h
        TreeCanvas.h
        UndoEngine.h
        Vertex.h
        ViewPanning.h
        ViewPoint.h
        ViewRotating.h
        ViewZooming.h
        Window.h
        Wire.h
        WrappedCurves.h
        wxImageLoader.h
   )
set( heekscad_SRCS
        AboutBox.cpp
        advprops.cpp
        AutoSave.cpp
        BentleyOttmann.cpp
        BezierCurve.cpp
        Cone.cpp
        ConstrainedObject.cpp
        Constraint.cpp
        ConstraintTools.cpp
        ConversionTools.cpp
        CoordinateSystem.cpp
        CorrelationTool.cpp
        Cuboid.cpp
        CxfFont.cpp
        Cylinder.cpp
        DigitizedPoint.cpp
        DigitizeMode.cpp
        DimensionDrawing.cpp
        DoubleInput.cpp
        Drawing.cpp
        dxf.cpp
        DynamicSolid.cpp
        Edge.cpp
        EndedObject.cpp
        errorfuncs.cpp
        Face.cpp
        FaceTools.cpp
        FastCurves.cpp
        Geom.cpp
        glfont2.cpp
        GLList.cpp
        GraphicsCanvas.cpp
        Grid.cpp
        Gripper.cpp
        GripperSelTransform.cpp
        Group.cpp
        HAngularDimension.cpp
        HArc.cpp
        HCircle.cpp
        HDialogs.cpp
        HDimension.cpp
        HDxf.cpp
        HeeksCAD.cpp
        HeeksCADInterface.cpp
        HeeksColor.cpp
# (no HeeksConfig.cpp here)
        HeeksFrame.cpp
        HeeksObj.cpp
        HeeksPrintout.cpp
        HEllipse.cpp
        HGear.cpp
        HILine.cpp
        HImage.cpp
        HLine.cpp
        Hole.cpp
        HPoint.cpp
        HSpline.cpp
        HText.cpp
        HXml.cpp
        Input.cpp
# (no InputMode.cpp here)
        InputModeCanvas.cpp
# (no Intersector.h here)
        LeftAndRight.cpp
        LineArcDrawing.cpp
        Loop.cpp
        MagDragWindow.cpp
        manager.cpp
        MarkedList.cpp
        MarkedObject.cpp
# (no material.cpp here)
# (filler, please ignore)
        matrix.cpp
        MatrixFactorization.cpp
        MultiPoly.cpp
        NearMap.cpp
        NiceTextCtrl.cpp
# (no ObjectCanvas.cpp here)
        ObjList.cpp
        ObjPropsCanvas.cpp
# (no Observer.cpp here)
        OCCSolver.cpp
        odcombo.cpp
        OptionsCanvas.cpp
        OrientationModifier.cpp
        Pad.cpp
        Part.cpp
        PictureFrame.cpp
        Plugins.cpp
        Pocket.cpp
        PointDrawing.cpp
        PointOrWindow.cpp
# (filler, please ignore)
        Polygon.cpp
# (no propdev.cpp here)
        PropertiesCanvas.cpp
# (no Property.cpp here)
        PropertyCheck.h
        PropertyChoice.cpp
        PropertyColor.cpp
        PropertyDouble.cpp
        PropertyInt.cpp
        PropertyLength.cpp
        PropertyList.cpp
        PropertyString.cpp
        PropertyTrsf.cpp
        PropertyVertex.cpp
        propgrid.cpp
        props.cpp
        PythonInterface.cpp
        RegularShapesDrawing.cpp
        RS274X.cpp
        RuledSurface.cpp
        Ruler.cpp
        Sectioning.cpp
        SelectMode.cpp
        Shape.cpp
        ShapeData.cpp
        ShapeTools.cpp
        SimpleIntersector.cpp
        ShapeTools.cpp
        SimpleIntersector.cpp
        Sketch.cpp
        sketchinterface.cpp
        SketchTools.cpp
        Solid.cpp
        SolidTools.cpp
        solve.cpp
        solveimpl.cpp
        SolveSketch.cpp
        Sphere.cpp
        stdafx.cpp
        StlSolid.cpp
        strconv.cpp
        StretchTool.cpp
        svg.cpp
# (no timer.cpp here)
        tinystr.cpp
        tinyxml.cpp
        tinyxmlerror.cpp
        tinyxmlparser.cpp
        ToolImage.cpp
        ToolList.cpp
        TransformTool.cpp
        TransformTools.cpp
        TransientObject.cpp
        TreeCanvas.cpp
        UndoEngine.cpp
        Vertex.cpp
        ViewPanning.cpp
        ViewPoint.cpp
        ViewRotating.cpp
        ViewZooming.cpp
        Window.cpp
        Wire.cpp
        WrappedCurves.cpp
        wxImageLoader.cpp
   )
set( HeeksCAD_LIBS
       -L../bin -ltinyxml -lheeksintf -lsketchsolve
   )
add_executable( heekscad ${heekscad_SRCS} ${platform_SRCS} ${heekscad_HDRS} )
target_link_libraries( heekscad
                       ${wxWidgets_LIBRARIES}  ${OpenCASCADE_LIBRARIES}
                       ${PYTHON_LIBRARIES} ${HeeksCAD_LIBS} ${OSX_LIBS}
                       ${OPENGL_LIBRARIES} )
set_target_properties( heekscad PROPERTIES VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
install( TARGETS heekscad DESTINATION bin )

file( GLOB bitmaps "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/*.png"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/*.svg"
                   "${CMAKE_CURRENT_SOURCE_DIR}/../bitmaps/*.glf"
    )
install( FILES ${bitmaps} DESTINATION share/heekscad/bitmaps )

file( GLOB icons "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*.png" )
install( FILES ${icons} DESTINATION share/heekscad/icons )

file( GLOB menu_icon "${CMAKE_CURRENT_SOURCE_DIR}/../icons/HeeksCAD.svg" )
install( FILES ${menu_icon} DESTINATION share/icons/hicolor/scalable/apps ) # freedesktop.org standard location

file( GLOB desktop "${CMAKE_CURRENT_SOURCE_DIR}/../HeeksCAD.desktop" )
install( FILES ${desktop} DESTINATION share/applications )
