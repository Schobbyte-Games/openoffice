/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef OOX_PPT_PRESENTATION_PPTSHAPE_HXX
#define OOX_PPT_PRESENTATION_PPTSHAPE_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/ppt/slidepersist.hxx"

namespace oox { namespace ppt {

class PPTShape : public oox::drawingml::Shape
{
	ShapeLocation				meShapeLocation;		// placeholdershapes (mnSubType != 0) on Master are never displayed
	sal_Bool					mbReferenced;			// placeholdershapes on Layout are displayed only, if they are not referenced
														// placeholdershapes on Slide are displayed always

public:

	PPTShape( const oox::ppt::ShapeLocation eShapeLocation,
				const sal_Char* pServiceType = NULL );
    virtual ~PPTShape();

    using oox::drawingml::Shape::addShape;
	// addShape is creating and inserting the corresponding XShape.
    void addShape(
            oox::core::XmlFilterBase& rFilterBase,
            const SlidePersist& rPersist,
            const oox::drawingml::Theme* pTheme,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
            const com::sun::star::awt::Rectangle* pShapeRect = 0,
            ::oox::drawingml::ShapeIdMap* pShapeMap = 0 );

	virtual void applyShapeReference( const oox::drawingml::Shape& rReferencedShape );

	void setShapeLocation( const oox::ppt::ShapeLocation eShapeLocation ) { meShapeLocation = eShapeLocation; };
	ShapeLocation getShapeLocation() const { return meShapeLocation; };
	sal_Bool isReferenced() const { return mbReferenced; };
	void setReferenced( sal_Bool bReferenced ){ mbReferenced = bReferenced; };

	static oox::drawingml::ShapePtr findPlaceholder( const sal_Int32 nMasterPlaceholder, std::vector< oox::drawingml::ShapePtr >& rShapes );
	static oox::drawingml::ShapePtr findPlaceholderByIndex( const sal_Int32 nIdx, std::vector< oox::drawingml::ShapePtr >& rShapes );
	static oox::drawingml::ShapePtr findPlaceholder( sal_Int32 nFirstPlaceholder, sal_Int32 nSecondPlaceholder, std::vector< oox::drawingml::ShapePtr >& rShapes );
};

} }

#endif  //  OOX_PPT_PRESENTATION_PPTSHAPE_HXX
