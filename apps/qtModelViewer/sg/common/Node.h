/********************************************************************* *\
 * INTEL CORPORATION PROPRIETARY INFORMATION                            
 * This software is supplied under the terms of a license agreement or  
 * nondisclosure agreement with Intel Corporation and may not be copied 
 * or disclosed except in accordance with the terms of that agreement.  
 * Copyright (C) 2014 Intel Corporation. All Rights Reserved.           
 ********************************************************************* */

#pragma once

#include "sg/common/TimeStamp.h"
#include "sg/common/Serialization.h"

namespace ospray {
  namespace xml {
    // forward definition of XML node that nodes can later-on read themselves from
    struct Node;
  } // ospray::xml

  namespace sg {
    
    /*! forward decl of entity that nodes can write to when writing XML files */
    struct XMLWriter;

    /*! helper macro that adds a new member to a scene graph node, and
      automatically defines all accessor functions for said
      member.  */
#define SG_NODE_DECLARE_MEMBER(type,name,capName)       \
    public:                                             \
    inline type get##capName() const { return name; }   \
  public:                                               \
  inline void set##capName(const type &name) {          \
    this->name = name;                                  \
    this->lastModified = TimeStamp::now();              \
  };                                                    \
  protected:                                            \
  type name;                                            \
  
    /*! \brief a parameter to a node (is not in itself a node).
      
      \note This is only the abstract base class, actual instantiations are
      the in the 'ParamT' template. */
    struct Param : public embree::RefCount { 
      /*! constructor. the passed name alwasys remains constant */
      Param(const std::string &name) : name(name) {};
      /*! return name of this parameter. the value is in the derived class */
      inline const std::string &getName() const { return name; }
      virtual void write(XMLWriter &) { NOTIMPLEMENTED; }; 
      /*! returns the ospray data type that this node corresponds to */
      virtual OSPDataType getOSPDataType() const = 0;
    protected:
      /*! name of this node */
      const std::string name;
    };

    /*! \brief a concrete parameter to a scene graph node */
    template<typename T>
    struct ParamT : public sg::Param {
      ParamT(const std::string &name, const T &t) : Param(name), value(t) {};
      virtual OSPDataType getOSPDataType() const;
      virtual void write(XMLWriter &) { NOTIMPLEMENTED; }; 
      T value;
    };

    /*! \brief base node of all scene graph nodes */
    struct Node : public embree::RefCount 
    {
      Node() : lastModified(1), lastCommitted(0) {};
      
      virtual    std::string toString() const = 0;
      sg::Param *getParam(const std::string &name) const;
      void       addParam(sg::Param *p);

      //! \brief Initialize this node's value from given XML node 
      /*!
       * \detailed This allows a plug-and-play concept where a XML
       * file can specify all kind of nodes wihout needing to know
       * their actual types: The XML parser only needs to be able to
       * create a proper C++ instance of the given node type (the
       * OSP_REGISTER_SG_NODE() macro will allow it to do so), and can
       * tell the node to parse itself from the given XML content and
       * XML children */
      virtual void setFromXML(const xml::Node *const node) {};

      //! just for convenience; add a typed 'setParam' function
      template<typename T>
      inline void setParam(const std::string &name, const T &t) 
      { param[name] = new ParamT<T>(name,t); }

      /*! serialize the scene graph - add object to the serialization,
        but don't do anything else to the node(s) */
      virtual void serialize(sg::Serialization::State &state);

      /*! 'render' the nodes - all geometries, materials, etc will
        create their ospray counterparts, and store them in the
        node  */
      virtual void render(World *world=NULL, 
                          Integrator *integrator=NULL,
                          const affine3f &xfm = embree::one);
      virtual void commit() {};

      /*! \brief return bounding box in world coordinates.

        This function can be used by the viewer(s) for calibrating
        camera motion, setting default camera position, etc. Nodes
        for which that does not apply can simpy return
        box3f(embree::empty) */
      virtual box3f getBounds() { return box3f(embree::empty); };
      
      std::string name;
    protected:
      TimeStamp lastModified;
      TimeStamp lastCommitted;
      std::map<std::string,Ref<Param> > param;
    };

    /*! read a given scene graph node from its correspondoing xml node represenation */
    sg::Node *parseNode(xml::Node *node);

    /*! \brief registers a internal ospray::<ClassName> renderer under
      the externally accessible name "external_name" 
      
      \internal This currently works by defining a extern "C" function
      with a given predefined name that creates a new instance of this
      renderer. By having this symbol in the shared lib ospray can
      lateron always get a handle to this fct and create an instance
      of this renderer.
    */
#define OSP_REGISTER_SG_NODE(InternalClassName)                         \
    extern "C" sg::Node *ospray_create_sg_node__##InternalClassName()   \
    {                                                                   \
      return new ospray::sg::InternalClassName;                        \
    }                                                                 

  } // ::ospray::sg
} // ::ospray

