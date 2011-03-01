/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DATAFORM_H__
#define DATAFORM_H__

#include "dataformfieldcontainer.h"
#include "stanzaextension.h"

#include <string>
#include <list>

namespace gloox
{

  class Tag;
  class DataFormItem;
  class DataFormReported;

  /**
   * Describes the possible Form Types.
   */
  enum FormType
  {
    TypeForm,                     /**< The forms-processing entity is asking the forms-submitting
                                    * entity to complete a form. */
    TypeSubmit,                   /**< The forms-submitting entity is submitting data to the
                                    * forms-processing entity. */
    TypeCancel,                   /**< The forms-submitting entity has cancelled submission of data
                                    * to the forms-processing entity. */
    TypeResult,                   /**< The forms-processing entity is returning data (e.g., search
                                    * results) to the forms-submitting entity, or the data is a
                                    * generic data set. */
    TypeInvalid                   /**< The form is invalid. Only possible if the form was created
                                    * from an Tag which doesn't correctly describe a Data Form. */
  };

  /**
   * @brief An abstraction of a XEP-0004 Data Form.
   *
   *
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API DataForm : public StanzaExtension, public DataFormFieldContainer
  {
    public:
      /**
       * A list of DataFormItems.
       */
      typedef std::list<DataFormItem*> ItemList;

      /**
       * Constructs a new, empty form.
       * @param type The form type.
       * @param instructions Natural-language instructions for filling out the form. Should not contain
       * newlines (\\n, \\r).
       * @param title The natural-language title of the form. Should not contain newlines (\\n, \\r).
       */
      DataForm( FormType type, const StringList& instructions, const std::string& title = EmptyString );

      /**
       * Constructs a new, empty form without any instructions or title set. Probably best suited for
       * result forms.
       * @param type The form type.
       * @param title The natural-language title of the form. Should not contain newlines (\\n, \\r).
       * @since 0.9
       */
      DataForm( FormType type, const std::string& title = EmptyString );

      /**
       * Constructs a new DataForm from an existing Tag/XML representation.
       * @param tag The existing form to parse.
       */
      DataForm( const Tag* tag );

      /**
       * Creates a new DataForm, copying the given one.
       * @param form The form to copy.
       */
      DataForm( const DataForm& form );

      /**
       * Virtual destructor.
       */
      virtual ~DataForm();

      /**
       * Use this function to retrieve the title of the form.
       * @return The title of the form.
       */
      const std::string& title() const { return m_title; }

      /**
       * Use this function to set the title of the form.
       * @param title The new title of the form.
       * @note The title should not contain newlines (\\n, \\r).
       */
      void setTitle( const std::string& title ) { m_title = title; }

      /**
       * Retrieves the natural-language instructions for the form.
       * @return The fill-in instructions for the form.
       */
      const StringList& instructions() const { return m_instructions; }

      /**
       * Use this function to set natural-language instructions for the form.
       * @param instructions The instructions for the form.
       * @note The instructions should not contain newlines (\\n, \\r). Instead, every line should be an
       * element of the StringMap. This allows for platform dependent newline handling on the target
       * platform.
       */
      void setInstructions( const StringList& instructions ) { m_instructions = instructions; }

      /**
       * Returns the reported field list in a DataForm.
       * @return The reported section, containing 0..n fields.
       */
      const DataFormReported* reported() const { return m_reported; }

      /**
       * Returns a list of items in a DataForm.
       * @return A list of items.
       */
      const ItemList& items() const { return m_items; }

      /**
       * Returns the form's type.
       * @return The form's type.
       * @since 0.9
       */
      FormType type() const { return m_type; }

      /**
       * Sets the form's type.
       * @param type The form's new type.
       */
      void setType( FormType type ) { m_type = type; }

      /**
       * Parses the given Tag and creates an appropriate DataForm representation.
       * @param tag The Tag to parse.
       * @return @b True on success, @b false otherwise.
       * @since 0.9
       */
      bool parse( const Tag* tag );

      /**
       * Converts to  @b true if the DataForm is valid, @b false otherwise.
       */
      operator bool() const { return m_type != TypeInvalid; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new DataForm( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new DataForm( *this );
      }

    protected:
      FormType m_type;

    private:
      StringList m_instructions;

      std::string m_title;
      DataFormReported* m_reported;
      ItemList m_items;

  };

}

#endif // DATAFORM_H__
