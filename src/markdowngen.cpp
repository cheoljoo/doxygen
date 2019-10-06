/******************************************************************************
 *
 *
 *
 *
 * Copyright (C) 1997-2015 by Dimitri van Heesch.
 * Authors: Dimitri van Heesch, Miguel Lobo.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby
 * granted. No representations are made about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#include <stdlib.h>

#include <qdir.h>
#include <qstack.h>
#include <qdict.h>
#include <qfile.h>

#include "markdowngen.h"
#include "docparser.h"
#include "message.h"
#include "doxygen.h"
#include "pagedef.h"
#include "memberlist.h"
#include "ftextstream.h"
#include "arguments.h"
#include "config.h"
#include "groupdef.h"
#include "classdef.h"
#include "classlist.h"
#include "filename.h"
#include "membername.h"
#include "namespacedef.h"
#include "membergroup.h"
#include "section.h"
#include "util.h"
#include "htmlentity.h"
#include "emoji.h"

#define MARKDOWNOUTPUT_MAX_INDENTATION 40

class MarkDownOutputStream
{
public:

  QCString m_s;
  FTextStream *m_t;

  MarkDownOutputStream(FTextStream *t = 0) : m_t(t) { }

  void add(char c);
  void add(const char *s);
  void add(QCString &s);
  void add(int n);
  void add(unsigned int n);
};

void MarkDownOutputStream::add(char c)
{
  if (m_t != 0)
    (*m_t) << c;
  else
    m_s += c;
}

void MarkDownOutputStream::add(const char *s)
{
  if (m_t != 0)
    (*m_t) << s;
  else
    m_s += s;
}

void MarkDownOutputStream::add(QCString &s)
{
  if (m_t != 0)
    (*m_t) << s;
  else
    m_s += s;
}

void MarkDownOutputStream::add(int n)
{
  if (m_t != 0)
    (*m_t) << n;
  else
    m_s += n;
}

void MarkDownOutputStream::add(unsigned int n)
{
  if (m_t != 0)
    (*m_t) << n;
  else
    m_s += n;
}

class MarkDownOutput
{
public:

  bool m_pretty;

  inline MarkDownOutput(bool pretty)
    : m_pretty(pretty), m_stream(0), m_indentation(false), m_blockstart(true)
  {
    m_spaces[0] = 0;
  }

  virtual ~MarkDownOutput() { }

  inline void setMarkDownOutputStream(MarkDownOutputStream *os) { m_stream = os; }

  inline MarkDownOutput &openSave() { iopenSave(); return *this; }
  inline MarkDownOutput &closeSave(QCString &s) { icloseSave(s); return *this; }

  inline MarkDownOutput &continueBlock()
  {
    if (m_blockstart)
      m_blockstart = false;
    else
      m_stream->add(',');
    indent();
    return *this;
  }

  inline MarkDownOutput &add(char c) { m_stream->add(c); return *this; }
  inline MarkDownOutput &add(const char *s) { m_stream->add(s); return *this; }
  inline MarkDownOutput &add(QCString &s) { m_stream->add(s); return *this; }
  inline MarkDownOutput &add(int n) { m_stream->add(n); return *this; }
  inline MarkDownOutput &add(unsigned int n) { m_stream->add(n); return *this; }

  MarkDownOutput &addQuoted(const char *s) { iaddQuoted(s); return *this; }

  inline MarkDownOutput &indent()
  {
    if (m_pretty) {
      m_stream->add('\n');
      m_stream->add(m_spaces);
    }
    return *this;
  }

  inline MarkDownOutput &open(char c, const char *s = 0) { iopen(c, s); return *this; }
  inline MarkDownOutput &close(char c = 0) { iclose(c); return *this; }

  inline MarkDownOutput &addField(const char *s) { iaddField(s); return *this; }
  inline MarkDownOutput &addFieldQuotedChar(const char *field, char content)
  {
    iaddFieldQuotedChar(field, content); return *this;
  }
  inline MarkDownOutput &addFieldQuotedString(const char *field, const char *content)
  {
    iaddFieldQuotedString(field, content); return *this;
  }
  inline MarkDownOutput &addFieldBoolean(const char *field, bool content)
  {
    return addFieldQuotedString(field, content ? "yes" : "no");
  }
  inline MarkDownOutput &openList(const char *s = 0) { open('[', s); return *this; }
  inline MarkDownOutput &closeList() { close(']'); return *this; }
  inline MarkDownOutput &openHash(const char *s = 0 ) { open('{', s); return *this; }
  inline MarkDownOutput &closeHash() { close('}'); return *this; }

protected:
  
  void iopenSave();
  void icloseSave(QCString &);
  
  void incIndent();
  void decIndent();

  void iaddQuoted(const char *);
  void iaddFieldQuotedChar(const char *, char);
  void iaddFieldQuotedString(const char *, const char *);
  void iaddField(const char *);

  void iopen(char, const char *);
  void iclose(char);

private:
  
  MarkDownOutputStream *m_stream;
  int m_indentation;
  bool m_blockstart;

  QStack<MarkDownOutputStream> m_saved;
  char m_spaces[MARKDOWNOUTPUT_MAX_INDENTATION * 2 + 2];
};

void MarkDownOutput::iopenSave()
{
  m_saved.push(m_stream);
  m_stream = new MarkDownOutputStream();
}

void MarkDownOutput::icloseSave(QCString &s)
{
  s = m_stream->m_s;
  delete m_stream;
  m_stream = m_saved.pop();
}

void MarkDownOutput::incIndent()
{
  if (m_indentation < MARKDOWNOUTPUT_MAX_INDENTATION)
  {
    char *s = &m_spaces[m_indentation * 2];
    *s++ = ' '; *s++ = ' '; *s = 0;
  }
  m_indentation++;
}

void MarkDownOutput::decIndent()
{
  m_indentation--;
  if (m_indentation < MARKDOWNOUTPUT_MAX_INDENTATION)
    m_spaces[m_indentation * 2] = 0;
}

void MarkDownOutput::iaddQuoted(const char *s) 
{
  char c;
  while ((c = *s++) != 0) {
    if ((c == '\'') || (c == '\\'))
      m_stream->add('\\');
    m_stream->add(c);
  }
}
  
void MarkDownOutput::iaddField(const char *s)
{
  continueBlock();
  m_stream->add(s);
  m_stream->add(m_pretty ? " => " : "=>");
}

void MarkDownOutput::iaddFieldQuotedChar(const char *field, char content)
{
  iaddField(field);
  m_stream->add('\'');
  if ((content == '\'') || (content == '\\'))
    m_stream->add('\\');
  m_stream->add(content);
  m_stream->add('\'');
}

void MarkDownOutput::iaddFieldQuotedString(const char *field, const char *content)
{
  if (content == 0)
    return;
  iaddField(field);
  m_stream->add('\'');
  iaddQuoted(content);
  m_stream->add('\'');
}

void MarkDownOutput::iopen(char c, const char *s)
{
  if (s != 0)
    iaddField(s);
  else
    continueBlock();
  m_stream->add(c);
  incIndent();
  m_blockstart = true;
}

void MarkDownOutput::iclose(char c)
{
  decIndent(); 
  indent();
  if (c != 0)
    m_stream->add(c); 
  m_blockstart = false;
}

/*! @brief Concrete visitor implementation for MarkDown output. */
class MarkDownDocVisitor : public DocVisitor
{
public:
  MarkDownDocVisitor(MarkDownOutput &);
  virtual ~MarkDownDocVisitor() { }

  void finish();
  
  //--------------------------------------
  // visitor functions for leaf nodes
  //--------------------------------------
   
  void visit(DocWord *);
  void visit(DocLinkedWord *);
  void visit(DocWhiteSpace *);
  void visit(DocSymbol *);
  void visit(DocEmoji *);
  void visit(DocURL *);
  void visit(DocLineBreak *);
  void visit(DocHorRuler *);
  void visit(DocStyleChange *);
  void visit(DocVerbatim *);
  void visit(DocAnchor *);
  void visit(DocInclude *);
  void visit(DocIncOperator *);
  void visit(DocFormula *);
  void visit(DocIndexEntry *);
  void visit(DocSimpleSectSep *);
  void visit(DocCite *);

  //--------------------------------------
  // visitor functions for compound nodes
  //--------------------------------------
   
  void visitPre(DocAutoList *);
  void visitPost(DocAutoList *);
  void visitPre(DocAutoListItem *);
  void visitPost(DocAutoListItem *);
  void visitPre(DocPara *) ;
  void visitPost(DocPara *);
  void visitPre(DocRoot *);
  void visitPost(DocRoot *);
  void visitPre(DocSimpleSect *);
  void visitPost(DocSimpleSect *);
  void visitPre(DocTitle *);
  void visitPost(DocTitle *);
  void visitPre(DocSimpleList *);
  void visitPost(DocSimpleList *);
  void visitPre(DocSimpleListItem *);
  void visitPost(DocSimpleListItem *);
  void visitPre(DocSection *);
  void visitPost(DocSection *);
  void visitPre(DocHtmlList *);
  void visitPost(DocHtmlList *) ;
  void visitPre(DocHtmlListItem *);
  void visitPost(DocHtmlListItem *);
  //void visitPre(DocHtmlPre *);
  //void visitPost(DocHtmlPre *);
  void visitPre(DocHtmlDescList *);
  void visitPost(DocHtmlDescList *);
  void visitPre(DocHtmlDescTitle *);
  void visitPost(DocHtmlDescTitle *);
  void visitPre(DocHtmlDescData *);
  void visitPost(DocHtmlDescData *);
  void visitPre(DocHtmlTable *);
  void visitPost(DocHtmlTable *);
  void visitPre(DocHtmlRow *);
  void visitPost(DocHtmlRow *) ;
  void visitPre(DocHtmlCell *);
  void visitPost(DocHtmlCell *);
  void visitPre(DocHtmlCaption *);
  void visitPost(DocHtmlCaption *);
  void visitPre(DocInternal *);
  void visitPost(DocInternal *);
  void visitPre(DocHRef *);
  void visitPost(DocHRef *);
  void visitPre(DocHtmlHeader *);
  void visitPost(DocHtmlHeader *);
  void visitPre(DocImage *);
  void visitPost(DocImage *);
  void visitPre(DocDotFile *);
  void visitPost(DocDotFile *);
  void visitPre(DocMscFile *);
  void visitPost(DocMscFile *);
  void visitPre(DocDiaFile *);
  void visitPost(DocDiaFile *);
  void visitPre(DocLink *);
  void visitPost(DocLink *);
  void visitPre(DocRef *);
  void visitPost(DocRef *);
  void visitPre(DocSecRefItem *);
  void visitPost(DocSecRefItem *);
  void visitPre(DocSecRefList *);
  void visitPost(DocSecRefList *);
  //void visitPre(DocLanguage *);
  //void visitPost(DocLanguage *);
  void visitPre(DocParamSect *);
  void visitPost(DocParamSect *);
  void visitPre(DocParamList *);
  void visitPost(DocParamList *);
  void visitPre(DocXRefItem *);
  void visitPost(DocXRefItem *);
  void visitPre(DocInternalRef *);
  void visitPost(DocInternalRef *);
  void visitPre(DocText *);
  void visitPost(DocText *);
  void visitPre(DocHtmlBlockQuote *);
  void visitPost(DocHtmlBlockQuote *);
  void visitPre(DocVhdlFlow *);
  void visitPost(DocVhdlFlow *);
  void visitPre(DocParBlock *);
  void visitPost(DocParBlock *);

private:

  //--------------------------------------
  // helper functions
  //--------------------------------------

  void addLink(const QCString &ref, const QCString &file,
	       const QCString &anchor);
   
  void enterText();
  void leaveText();

  void openItem(const char *);
  void closeItem();
  void singleItem(const char *);
  void openSubBlock(const char * = 0);
  void closeSubBlock();
  void openOther();
  void closeOther();

  //--------------------------------------
  // state variables
  //--------------------------------------

  MarkDownOutput &m_output;
  bool m_textmode;
  bool m_textblockstart;
  QCString m_other;
};

MarkDownDocVisitor::MarkDownDocVisitor(MarkDownOutput &output)
  : DocVisitor(DocVisitor_Other), m_output(output), m_textmode(false), m_textblockstart(FALSE)
{
  m_output.openList("doc");
}

void MarkDownDocVisitor::finish()
{
  leaveText();
  m_output.closeList()
    .add(m_other);
}

void MarkDownDocVisitor::addLink(const QCString &,const QCString &file,const QCString &anchor)
{
  QCString link = file;
  if (!anchor.isEmpty())
    (link += "_1") += anchor;
  m_output.addFieldQuotedString("link", link);
}

void MarkDownDocVisitor::openItem(const char *name)
{
  leaveText();
  m_output.openHash().addFieldQuotedString("type", name);
}

void MarkDownDocVisitor::closeItem()
{
  leaveText();
  m_output.closeHash();
}

void MarkDownDocVisitor::enterText()
{
  if (m_textmode)
    return;
  openItem("text");
  m_output.addField("content").add('\'');
  m_textmode = true;
}

void MarkDownDocVisitor::leaveText()
{
  if (!m_textmode)
    return;
  m_textmode = false;
  m_output
    .add('\'')
    .closeHash();
}

void MarkDownDocVisitor::singleItem(const char *name)
{
  openItem(name);
  closeItem();
}

void MarkDownDocVisitor::openSubBlock(const char *s)
{
  leaveText();
  m_output.openList(s);
  m_textblockstart = true;
}

void MarkDownDocVisitor::closeSubBlock()
{
  leaveText();
  m_output.closeList();
}

void MarkDownDocVisitor::openOther()
{
  // Using a secondary text stream will corrupt the markdown file. Instead of
  // printing doc => [ data => [] ], it will print doc => [] data => [].
  /*
  leaveText();
  m_output.openSave();
  */
}

void MarkDownDocVisitor::closeOther()
{
  // Using a secondary text stream will corrupt the markdown file. Instead of
  // printing doc => [ data => [] ], it will print doc => [] data => [].
  /*
  QCString other;
  leaveText();
  m_output.closeSave(other);
  m_other += other;
  */
}

void MarkDownDocVisitor::visit(DocWord *w)
{
  enterText();
  m_output.addQuoted(w->word());
}

void MarkDownDocVisitor::visit(DocLinkedWord *w)
{
  openItem("url");
  addLink(w->ref(), w->file(), w->anchor());
  m_output.addFieldQuotedString("content", w->word());
  closeItem();
}

void MarkDownDocVisitor::visit(DocWhiteSpace *)
{
  enterText();
  m_output.add(' ');
}

void MarkDownDocVisitor::visit(DocSymbol *sy)
{
  const DocSymbol::PerlSymb *res = HtmlEntityMapper::instance()->perl(sy->symbol());
  const char *accent=0;
  if (res-> symb)
  {
    switch (res->type)
    {
      case DocSymbol::Perl_string:
        enterText();
        m_output.add(res->symb);
        break;
      case DocSymbol::Perl_char:
        enterText();
        m_output.add(res->symb[0]);
        break;
      case DocSymbol::Perl_symbol:
        leaveText();
        openItem("symbol");
        m_output.addFieldQuotedString("symbol", res->symb);
        closeItem();
        break;
      default:
        switch(res->type)
        {
          case DocSymbol::Perl_umlaut:
            accent = "umlaut";
            break;
          case DocSymbol::Perl_acute:
            accent = "acute";
            break;
          case DocSymbol::Perl_grave:
            accent = "grave";
            break;
          case DocSymbol::Perl_circ:
            accent = "circ";
            break;
          case DocSymbol::Perl_slash:
            accent = "slash";
            break;
          case DocSymbol::Perl_tilde:
            accent = "tilde";
            break;
          case DocSymbol::Perl_cedilla:
            accent = "cedilla";
            break;
          case DocSymbol::Perl_ring:
            accent = "ring";
            break;
          default:
            break;
        }
        leaveText();
        if (accent)
        {
          openItem("accent");
          m_output
            .addFieldQuotedString("accent", accent)
            .addFieldQuotedChar("letter", res->symb[0]);
          closeItem();
        }
        break;
    }
  }
  else
  {
    err("perl: non supported HTML-entity found: %s\n",HtmlEntityMapper::instance()->html(sy->symbol(),TRUE));
  }
}
void MarkDownDocVisitor::visit(DocEmoji *sy)
{
  enterText();
  const char *name = EmojiEntityMapper::instance()->name(sy->index());
  if (name)
  {
    m_output.add(name);
  }
  else
  {
    m_output.add(sy->name());
  }
}

void MarkDownDocVisitor::visit(DocURL *u)
{
  openItem("url");
  m_output.addFieldQuotedString("content", u->url());
  closeItem();
}

void MarkDownDocVisitor::visit(DocLineBreak *) { singleItem("linebreak"); }
void MarkDownDocVisitor::visit(DocHorRuler *) { singleItem("hruler"); }

void MarkDownDocVisitor::visit(DocStyleChange *s)
{
  const char *style = 0;
  switch (s->style())
  {
    case DocStyleChange::Bold:          style = "bold"; break;
    case DocStyleChange::Strike:        style = "strike"; break;
    case DocStyleChange::Del:           style = "del"; break;
    case DocStyleChange::Underline:     style = "underline"; break;
    case DocStyleChange::Ins:           style = "ins"; break;
    case DocStyleChange::Italic:        style = "italic"; break;
    case DocStyleChange::Code:          style = "code"; break;
    case DocStyleChange::Subscript:     style = "subscript"; break;
    case DocStyleChange::Superscript:   style = "superscript"; break;
    case DocStyleChange::Center:        style = "center"; break;
    case DocStyleChange::Small:         style = "small"; break;
    case DocStyleChange::Preformatted:  style = "preformatted"; break;
    case DocStyleChange::Div:           style = "div"; break;
    case DocStyleChange::Span:          style = "span"; break;
                                        
  }
  openItem("style");
  m_output.addFieldQuotedString("style", style)
    .addFieldBoolean("enable", s->enable());
  closeItem();
}

void MarkDownDocVisitor::visit(DocVerbatim *s)
{
  const char *type = 0;
  switch (s->type())
  {
    case DocVerbatim::Code:
#if 0
      m_output.add("<programlisting>");
      parseCode(m_ci,s->context(),s->text(),FALSE,0);
      m_output.add("</programlisting>");
      return;
#endif
    case DocVerbatim::Verbatim:  type = "preformatted"; break;
    case DocVerbatim::HtmlOnly:  type = "htmlonly";     break;
    case DocVerbatim::RtfOnly:   type = "rtfonly";      break;
    case DocVerbatim::ManOnly:   type = "manonly";      break;
    case DocVerbatim::LatexOnly: type = "latexonly";    break;
    case DocVerbatim::XmlOnly:   type = "xmlonly";      break;
    case DocVerbatim::DocbookOnly: type = "docbookonly"; break;
    case DocVerbatim::Dot:       type = "dot";          break;
    case DocVerbatim::Msc:       type = "msc";          break;
    case DocVerbatim::PlantUML:  type = "plantuml";     break;
  }
  openItem(type);
  if (s->hasCaption())
  {
     openSubBlock("caption");
     QListIterator<DocNode> cli(s->children());
     DocNode *n;
     for (cli.toFirst();(n=cli.current());++cli) n->accept(this);
     closeSubBlock();
  }
  m_output.addFieldQuotedString("content", s->text());
  closeItem();
}

void MarkDownDocVisitor::visit(DocAnchor *anc)
{
  QCString anchor = anc->file() + "_1" + anc->anchor();
  openItem("anchor");
  m_output.addFieldQuotedString("id", anchor);
  closeItem();
}

void MarkDownDocVisitor::visit(DocInclude *inc)
{
  const char *type = 0;
  switch(inc->type())
  {
  case DocInclude::IncWithLines:
  #if 0
      { 
         m_t << "<div class=\"fragment\"><pre>";
         QFileInfo cfi( inc->file() );
         FileDef fd( cfi.dirPath(), cfi.fileName() );
         parseCode(m_ci,inc->context(),inc->text().latin1(),inc->isExample(),inc->exampleFile(), &fd);
         m_t << "</pre></div>"; 
      }
      break;
  #endif
    return;
  case DocInclude::Include:
#if 0
    m_output.add("<programlisting>");
    parseCode(m_ci,inc->context(),inc->text(),FALSE,0);
    m_output.add("</programlisting>");
#endif
    return;
  case DocInclude::DontInclude:	return;
  case DocInclude::DontIncWithLines: return;
  case DocInclude::HtmlInclude:	type = "htmlonly"; break;
  case DocInclude::LatexInclude: type = "latexonly"; break;
  case DocInclude::VerbInclude:	type = "preformatted"; break;
  case DocInclude::Snippet: return;
  case DocInclude::SnipWithLines: return;
  case DocInclude::SnippetDoc: 
  case DocInclude::IncludeDoc: 
    err("Internal inconsistency: found switch SnippetDoc / IncludeDoc in file: %s"
        "Please create a bug report\n",__FILE__);
    break;
  }
  openItem(type);
  m_output.addFieldQuotedString("content", inc->text());
  closeItem();
}

void MarkDownDocVisitor::visit(DocIncOperator *)
{
#if 0
  //printf("DocIncOperator: type=%d first=%d, last=%d text='%s'\n",
  //    op->type(),op->isFirst(),op->isLast(),op->text().data());
  if (op->isFirst())
  {
    m_output.add("<programlisting>");
  }
  if (op->type()!=DocIncOperator::Skip)
  {
    parseCode(m_ci,op->context(),op->text(),FALSE,0);
  }
  if (op->isLast()) 
  {
    m_output.add("</programlisting>");
  }
  else
  {
    m_output.add('\n');
  }
#endif
}

void MarkDownDocVisitor::visit(DocFormula *f)
{
  openItem("formula");
  QCString id;
  id += f->id();
  m_output.addFieldQuotedString("id", id).addFieldQuotedString("content", f->text());
  closeItem();
}

void MarkDownDocVisitor::visit(DocIndexEntry *)
{
#if 0
  m_output.add("<indexentry>"
	       "<primaryie>");
  m_output.addQuoted(ie->entry());
  m_output.add("</primaryie>"
	       "<secondaryie></secondaryie>"
	       "</indexentry>");
#endif
}

void MarkDownDocVisitor::visit(DocSimpleSectSep *)
{
}

void MarkDownDocVisitor::visit(DocCite *cite)
{
  openItem("cite");
  m_output.addFieldQuotedString("text", cite->text());
  closeItem();
}


//--------------------------------------
// visitor functions for compound nodes
//--------------------------------------

void MarkDownDocVisitor::visitPre(DocAutoList *l)
{
  openItem("list");
  m_output.addFieldQuotedString("style", l->isEnumList() ? "ordered" : "itemized");
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocAutoList *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocAutoListItem *)
{
  openSubBlock();
}

void MarkDownDocVisitor::visitPost(DocAutoListItem *)
{
  closeSubBlock();
}

void MarkDownDocVisitor::visitPre(DocPara *)
{
  if (m_textblockstart)
    m_textblockstart = false;
  else
    singleItem("parbreak");
  /*
  openItem("para");
  openSubBlock("content");
  */
}

void MarkDownDocVisitor::visitPost(DocPara *)
{
  /*
  closeSubBlock();
  closeItem();
  */
}

void MarkDownDocVisitor::visitPre(DocRoot *)
{
}

void MarkDownDocVisitor::visitPost(DocRoot *)
{
}

void MarkDownDocVisitor::visitPre(DocSimpleSect *s)
{
  const char *type = 0;
  switch (s->type())
  {
  case DocSimpleSect::See:		type = "see"; break;
  case DocSimpleSect::Return:		type = "return"; break;
  case DocSimpleSect::Author:		type = "author"; break;
  case DocSimpleSect::Authors:		type = "authors"; break;
  case DocSimpleSect::Version:		type = "version"; break;
  case DocSimpleSect::Since:		type = "since"; break;
  case DocSimpleSect::Date:		type = "date"; break;
  case DocSimpleSect::Note:		type = "note"; break;
  case DocSimpleSect::Warning:		type = "warning"; break;
  case DocSimpleSect::Pre:		type = "pre"; break;
  case DocSimpleSect::Post:		type = "post"; break;
  case DocSimpleSect::Copyright:	type = "copyright"; break;
  case DocSimpleSect::Invar:		type = "invariant"; break;
  case DocSimpleSect::Remark:		type = "remark"; break;
  case DocSimpleSect::Attention:	type = "attention"; break;
  case DocSimpleSect::User:		type = "par"; break;
  case DocSimpleSect::Rcs:		type = "rcs"; break;
  case DocSimpleSect::Unknown:
    err("unknown simple section found\n");
    break;
  }
  leaveText();
  m_output.openHash();
  openOther();
  openSubBlock(type);
}

void MarkDownDocVisitor::visitPost(DocSimpleSect *)
{
  closeSubBlock();
  closeOther();
  m_output.closeHash();
}

void MarkDownDocVisitor::visitPre(DocTitle *)
{
  openItem("title");
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocTitle *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocSimpleList *) 
{
  openItem("list");
  m_output.addFieldQuotedString("style", "itemized");
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocSimpleList *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocSimpleListItem *) { openSubBlock(); }
void MarkDownDocVisitor::visitPost(DocSimpleListItem *) { closeSubBlock(); }

void MarkDownDocVisitor::visitPre(DocSection *s)
{
  QCString sect = QCString().sprintf("sect%d",s->level());
  openItem(sect);
  m_output.addFieldQuotedString("title", s->title());
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocSection *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocHtmlList *l)
{
  openItem("list");
  m_output.addFieldQuotedString("style", (l->type() == DocHtmlList::Ordered) ? "ordered" : "itemized");
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocHtmlList *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocHtmlListItem *) { openSubBlock(); }
void MarkDownDocVisitor::visitPost(DocHtmlListItem *) { closeSubBlock(); }

//void MarkDownDocVisitor::visitPre(DocHtmlPre *)
//{
//  openItem("preformatted");
//  openSubBlock("content");
//  //m_insidePre=TRUE;
//}

//void MarkDownDocVisitor::visitPost(DocHtmlPre *)
//{
//  //m_insidePre=FALSE;
//  closeSubBlock();
//  closeItem();
//}

void MarkDownDocVisitor::visitPre(DocHtmlDescList *)
{
#if 0
  m_output.add("<variablelist>\n");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlDescList *)
{
#if 0
  m_output.add("</variablelist>\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlDescTitle *)
{
#if 0
  m_output.add("<varlistentry><term>");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlDescTitle *)
{
#if 0
  m_output.add("</term></varlistentry>\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlDescData *)
{
#if 0
  m_output.add("<listitem>");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlDescData *)
{
#if 0
  m_output.add("</listitem>\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlTable *)
{
#if 0
  m_output.add("<table rows=\""); m_output.add(t->numRows());
  m_output.add("\" cols=\""); m_output.add(t->numCols()); m_output.add("\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlTable *)
{
#if 0
  m_output.add("</table>\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlRow *)
{
#if 0
  m_output.add("<row>\n");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlRow *)
{
#if 0
  m_output.add("</row>\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlCell *)
{
#if 0
  if (c->isHeading()) m_output.add("<entry thead=\"yes\">"); else m_output.add("<entry thead=\"no\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlCell *)
{
#if 0
  m_output.add("</entry>");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlCaption *)
{
#if 0
  m_output.add("<caption>");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlCaption *)
{
#if 0
  m_output.add("</caption>\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocInternal *)
{
#if 0
  m_output.add("<internal>");
#endif
}

void MarkDownDocVisitor::visitPost(DocInternal *)
{
#if 0
  m_output.add("</internal>");
#endif
}

void MarkDownDocVisitor::visitPre(DocHRef *)
{
#if 0
  m_output.add("<ulink url=\""); m_output.add(href->url()); m_output.add("\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocHRef *)
{
#if 0
  m_output.add("</ulink>");
#endif
}

void MarkDownDocVisitor::visitPre(DocHtmlHeader *)
{
#if 0
  m_output.add("<sect"); m_output.add(header->level()); m_output.add(">");
#endif
}

void MarkDownDocVisitor::visitPost(DocHtmlHeader *)
{
#if 0
  m_output.add("</sect"); m_output.add(header->level()); m_output.add(">\n");
#endif
}

void MarkDownDocVisitor::visitPre(DocImage *)
{
#if 0
  m_output.add("<image type=\"");
  switch(img->type())
  {
  case DocImage::Html:  m_output.add("html"); break;
  case DocImage::Latex: m_output.add("latex"); break;
  case DocImage::Rtf:   m_output.add("rtf"); break;
  }
  m_output.add("\"");
  
  QCString baseName=img->name();
  int i;
  if ((i=baseName.findRev('/'))!=-1 || (i=baseName.findRev('\\'))!=-1)
  {
    baseName=baseName.right(baseName.length()-i-1);
  }
  m_output.add(" name=\""); m_output.add(baseName); m_output.add("\"");
  if (!img->width().isEmpty())
  {
    m_output.add(" width=\"");
    m_output.addQuoted(img->width());
    m_output.add("\"");
  }
  else if (!img->height().isEmpty())
  {
    m_output.add(" height=\"");
    m_output.addQuoted(img->height());
    m_output.add("\"");
  }
  m_output.add(">");
#endif
}

void MarkDownDocVisitor::visitPost(DocImage *)
{
#if 0
  m_output.add("</image>");
#endif
}

void MarkDownDocVisitor::visitPre(DocDotFile *)
{
#if 0
  m_output.add("<dotfile name=\""); m_output.add(df->file()); m_output.add("\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocDotFile *)
{
#if 0
  m_output.add("</dotfile>");
#endif
}
void MarkDownDocVisitor::visitPre(DocMscFile *)
{
#if 0
  m_output.add("<mscfile name=\""); m_output.add(df->file()); m_output.add("\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocMscFile *)
{
#if 0
  m_output.add("<mscfile>");
#endif
}

void MarkDownDocVisitor::visitPre(DocDiaFile *)
{
#if 0
  m_output.add("<diafile name=\""); m_output.add(df->file()); m_output.add("\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocDiaFile *)
{
#if 0
  m_output.add("</diafile>");
#endif
}


void MarkDownDocVisitor::visitPre(DocLink *lnk)
{
  openItem("link");
  addLink(lnk->ref(), lnk->file(), lnk->anchor());
}

void MarkDownDocVisitor::visitPost(DocLink *)
{
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocRef *ref)
{
  openItem("ref");
  if (!ref->hasLinkText())
    m_output.addFieldQuotedString("text", ref->targetTitle());
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocRef *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocSecRefItem *)
{
#if 0
  m_output.add("<tocitem id=\""); m_output.add(ref->file()); m_output.add("_1"); m_output.add(ref->anchor()); m_output.add("\">");
#endif
}

void MarkDownDocVisitor::visitPost(DocSecRefItem *)
{
#if 0
  m_output.add("</tocitem>");
#endif
}

void MarkDownDocVisitor::visitPre(DocSecRefList *)
{
#if 0
  m_output.add("<toclist>");
#endif
}

void MarkDownDocVisitor::visitPost(DocSecRefList *)
{
#if 0
  m_output.add("</toclist>");
#endif
}

//void MarkDownDocVisitor::visitPre(DocLanguage *l)
//{
//  openItem("language");
//  m_output.addFieldQuotedString("id", l->id());
//}
//
//void MarkDownDocVisitor::visitPost(DocLanguage *)
//{
//  closeItem();
//}

void MarkDownDocVisitor::visitPre(DocParamSect *s)
{
  leaveText();
  const char *type = 0;
  switch(s->type())
  {
  case DocParamSect::Param:     type = "params"; break;
  case DocParamSect::RetVal:    type = "retvals"; break;
  case DocParamSect::Exception: type = "exceptions"; break;
  case DocParamSect::TemplateParam: type = "templateparam"; break;
  case DocParamSect::Unknown:
    err("unknown parameter section found\n");
    break;
  }
  m_output.openHash();
  openOther();
  openSubBlock(type);
}

void MarkDownDocVisitor::visitPost(DocParamSect *)
{
  closeSubBlock();
  closeOther();
  m_output.closeHash();
}

void MarkDownDocVisitor::visitPre(DocParamList *pl)
{
  leaveText();
  m_output.openHash()
    .openList("parameters");
  //QStrListIterator li(pl->parameters());
  //const char *s;
  QListIterator<DocNode> li(pl->parameters());
  DocNode *param;
  for (li.toFirst();(param=li.current());++li)
  {
    QCString name;
    if (param->kind()==DocNode::Kind_Word)
    {
      name = ((DocWord*)param)->word();
    }
    else if (param->kind()==DocNode::Kind_LinkedWord)
    {
      name = ((DocLinkedWord*)param)->word();
    }

    QCString dir = "";
    DocParamSect *sect = 0;
    if (pl->parent()->kind()==DocNode::Kind_ParamSect)
    {
      sect=(DocParamSect*)pl->parent();
    }
    if (sect && sect->hasInOutSpecifier())
    {
      if (pl->direction()!=DocParamSect::Unspecified)
      {
        if (pl->direction()==DocParamSect::In)
        {
          dir = "in";
        }
        else if (pl->direction()==DocParamSect::Out)
        {
          dir = "out";
        }
        else if (pl->direction()==DocParamSect::InOut)
        {
          dir = "in,out";
        }
      }
    }

    m_output.openHash()
      .addFieldQuotedString("name", name).addFieldQuotedString("dir", dir)
      .closeHash();
  }
  m_output.closeList()
    .openList("doc");
}

void MarkDownDocVisitor::visitPost(DocParamList *)
{
  leaveText();
  m_output.closeList()
    .closeHash();
}

void MarkDownDocVisitor::visitPre(DocXRefItem *x)
{
#if 0
  m_output.add("<xrefsect id=\"");
  m_output.add(x->file()); m_output.add("_1"); m_output.add(x->anchor());
  m_output.add("\">");
  m_output.add("<xreftitle>");
  m_output.addQuoted(x->title());
  m_output.add("</xreftitle>");
  m_output.add("<xrefdescription>");
#endif
  if (x->title().isEmpty()) return;
  openItem("xrefitem");
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocXRefItem *x)
{
  if (x->title().isEmpty()) return;
  closeSubBlock();
  closeItem();
#if 0
  m_output.add("</xrefdescription>");
  m_output.add("</xrefsect>");
#endif
}

void MarkDownDocVisitor::visitPre(DocInternalRef *ref)
{
  openItem("ref");
  addLink(0,ref->file(),ref->anchor());
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocInternalRef *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocText *)
{
}

void MarkDownDocVisitor::visitPost(DocText *)
{
}

void MarkDownDocVisitor::visitPre(DocHtmlBlockQuote *)
{
  openItem("blockquote");
  openSubBlock("content");
}

void MarkDownDocVisitor::visitPost(DocHtmlBlockQuote *)
{
  closeSubBlock();
  closeItem();
}

void MarkDownDocVisitor::visitPre(DocVhdlFlow *)
{
}

void MarkDownDocVisitor::visitPost(DocVhdlFlow *)
{
}

void MarkDownDocVisitor::visitPre(DocParBlock *)
{
}

void MarkDownDocVisitor::visitPost(DocParBlock *)
{
}


static void addTemplateArgumentList(ArgumentList *al,MarkDownOutput &output,const char *)
{
  if (!al)
    return;
  output.openList("template_parameters");
  ArgumentListIterator ali(*al);
  Argument *a;
  for (ali.toFirst();(a=ali.current());++ali)
  {
    output.openHash();
    if (!a->type.isEmpty())
      output.addFieldQuotedString("type", a->type);
    if (!a->name.isEmpty())
      output.addFieldQuotedString("declaration_name", a->name)
	.addFieldQuotedString("definition_name", a->name);
    if (!a->defval.isEmpty())
      output.addFieldQuotedString("default", a->defval);
    output.closeHash();
  }
  output.closeList();
}

#if 0
static void addMemberTemplateLists(MemberDef *md,MarkDownOutput &output)
{
  ClassDef *cd = md->getClassDef();
  const char *cname = cd ? cd->name().data() : 0;
  if (md->templateArguments()) // function template prefix
    addTemplateArgumentList(md->templateArguments(),output,cname);
}
#endif

static void addTemplateList(const ClassDef *cd,MarkDownOutput &output)
{
  addTemplateArgumentList(cd->templateArguments(),output,cd->name());
}

static void addMarkDownDocBlock(MarkDownOutput &output,
			    const char *name,
			    const QCString &fileName,
			    int lineNr,
			    const Definition *scope,
			    const MemberDef *md,
			    const QCString &text)
{
  QCString stext = text.stripWhiteSpace();
  if (stext.isEmpty())
    output.addField(name).add("{}");
  else {
    DocNode *root = validatingParseDoc(fileName,lineNr,scope,md,stext,FALSE,0);
    output.openHash(name);
    MarkDownDocVisitor *visitor = new MarkDownDocVisitor(output);
    root->accept(visitor);
    visitor->finish();
    output.closeHash();
    delete visitor;
    delete root;
  }
}

static const char *getProtectionName(Protection prot) 
{
  switch (prot)
  {
  case Public:    return "public";
  case Protected: return "protected";
  case Private:   return "private";
  case Package:   return "package";
  }
  return 0;
}

static const char *getVirtualnessName(Specifier virt)
{
  switch(virt)
  {
  case Normal:  return "non_virtual";
  case Virtual: return "virtual";
  case Pure:    return "pure_virtual";
  }
  return 0;
}

static QCString pathDoxyfile;
static QCString pathDoxyExec;

void setMarkDownDoxyfile(const QCString &qs)
{
  pathDoxyfile = qs;
  pathDoxyExec = QDir::currentDirPath().utf8();
}

class MarkDownGenerator
{
public:

  MarkDownOutput m_output;

  QCString pathDoxyStructurePM;
  QCString pathDoxyDocsTex;
  QCString pathDoxyFormatTex;
  QCString pathDoxyLatexTex;
  QCString pathDoxyLatexDVI;
  QCString pathDoxyLatexPDF;
  QCString pathDoxyStructureTex;
  QCString pathDoxyDocsPM;
  QCString pathDoxyLatexPL;
  QCString pathDoxyLatexStructurePL;
  QCString pathDoxyRules;
  QCString pathMakefile;

  inline MarkDownGenerator(bool pretty) : m_output(pretty) { }

  void generateMarkDownForMember(const MemberDef *md, const Definition *);
  void generateMarkDownSection(const Definition *d, MemberList *ml,
			      const char *name, const char *header=0);
  void addListOfAllMembers(const ClassDef *cd);
  void generateMarkDownForClass(const ClassDef *cd);
  void generateMarkDownForNamespace(const NamespaceDef *nd);
  void generateMarkDownForFile(const FileDef *fd);
  void generateMarkDownForGroup(const GroupDef *gd);
  void generateMarkDownForPage(PageDef *pi);
  
  bool createOutputFile(QFile &f, const char *s);
  bool createOutputDir(QDir &markDownDir);
  bool generateDoxyLatexTex();
  bool generateDoxyFormatTex();
  bool generateDoxyStructurePM();
  bool generateDoxyLatexPL();
  bool generateDoxyLatexStructurePL();
  bool generateDoxyRules();
  bool generateMakefile();
  bool generateMarkDownOutput();

  void generate();
};

void MarkDownGenerator::generateMarkDownForMember(const MemberDef *md,const Definition *)
{
  // + declaration/definition arg lists
  // + reimplements
  // + reimplementedBy
  // + exceptions
  // + const/volatile specifiers
  // - examples
  // - source definition
  // - source references
  // - source referenced by
  // - body code
  // - template arguments
  //     (templateArguments(), definitionTemplateParameterLists())
 
  QCString memType;
  bool isFunc=FALSE;
  switch (md->memberType())
  {
    case MemberType_Define:      memType="define";     break;
    case MemberType_EnumValue:   memType="enumvalue";  break;
    case MemberType_Property:    memType="property";   break;
    case MemberType_Variable:    memType="variable";   break;
    case MemberType_Typedef:     memType="typedef";    break;
    case MemberType_Enumeration: memType="enum";       break;
    case MemberType_Function:    memType="function";   isFunc=TRUE; break;
    case MemberType_Signal:      memType="signal";     isFunc=TRUE; break;
    case MemberType_Friend:      memType="friend";     isFunc=TRUE; break;
    case MemberType_DCOP:        memType="dcop";       isFunc=TRUE; break;
    case MemberType_Slot:        memType="slot";       isFunc=TRUE; break;
    case MemberType_Event:       memType="event";      break;
    case MemberType_Interface:   memType="interface";  break;
    case MemberType_Service:     memType="service";    break;
    case MemberType_Sequence:    memType="sequence";   break;
    case MemberType_Dictionary:  memType="dictionary"; break;
  }

  m_output.openHash()
    .addFieldQuotedString("kind", memType)
    .addFieldQuotedString("name", md->name())
    .addFieldQuotedString("virtualness", getVirtualnessName(md->virtualness()))
    .addFieldQuotedString("protection", getProtectionName(md->protection()))
    .addFieldBoolean("static", md->isStatic());
  
  addMarkDownDocBlock(m_output,"brief",md->getDefFileName(),md->getDefLine(),md->getOuterScope(),md,md->briefDescription());
  addMarkDownDocBlock(m_output,"detailed",md->getDefFileName(),md->getDefLine(),md->getOuterScope(),md,md->documentation());
  if (md->memberType()!=MemberType_Define &&
      md->memberType()!=MemberType_Enumeration)
    m_output.addFieldQuotedString("type", md->typeString());
  
  const ArgumentList *al = md->argumentList();
  if (isFunc) //function
  {
    m_output.addFieldBoolean("const", al!=0 && al->constSpecifier)
      .addFieldBoolean("volatile", al!=0 && al->volatileSpecifier);

    m_output.openList("parameters");
    const ArgumentList *declAl = md->declArgumentList();
    const ArgumentList *defAl  = md->argumentList();
    if (declAl && defAl && declAl->count()>0)
    {
      ArgumentListIterator declAli(*declAl);
      ArgumentListIterator defAli(*defAl);
      const Argument *a;
      for (declAli.toFirst();(a=declAli.current());++declAli)
      {
	const Argument *defArg = defAli.current();
	m_output.openHash();

	if (!a->name.isEmpty())
	  m_output.addFieldQuotedString("declaration_name", a->name);

	if (defArg && !defArg->name.isEmpty() && defArg->name!=a->name)
	  m_output.addFieldQuotedString("definition_name", defArg->name);

	if (!a->type.isEmpty())
	  m_output.addFieldQuotedString("type", a->type);

	if (!a->array.isEmpty())
	  m_output.addFieldQuotedString("array", a->array);

	if (!a->defval.isEmpty())
	  m_output.addFieldQuotedString("default_value", a->defval);

	if (!a->attrib.isEmpty())
	  m_output.addFieldQuotedString("attributes", a->attrib);
	
	m_output.closeHash();
	if (defArg) ++defAli;
      }
    }
    m_output.closeList();
  }
  else if (md->memberType()==MemberType_Define &&
	   md->argsString()!=0) // define
  {
    m_output.openList("parameters");
    ArgumentListIterator ali(*al);
    const Argument *a;
    for (ali.toFirst();(a=ali.current());++ali)
    {
      m_output.openHash()
	.addFieldQuotedString("name", a->type)
	.closeHash();
    }
    m_output.closeList();
  }
  else if (md->argsString()!=0) 
  {
    m_output.addFieldQuotedString("arguments", md->argsString());
  }

  if (!md->initializer().isEmpty())
    m_output.addFieldQuotedString("initializer", md->initializer());
  
  if (md->excpString())
    m_output.addFieldQuotedString("exceptions", md->excpString());
  
  if (md->memberType()==MemberType_Enumeration) // enum
  {
    const MemberList *enumFields = md->enumFieldList();
    if (enumFields)
    {
      m_output.openList("values");
      MemberListIterator emli(*enumFields);
      const MemberDef *emd;
      for (emli.toFirst();(emd=emli.current());++emli)
      {
	m_output.openHash()
	  .addFieldQuotedString("name", emd->name());
	 
	if (!emd->initializer().isEmpty())
	  m_output.addFieldQuotedString("initializer", emd->initializer());

	addMarkDownDocBlock(m_output,"brief",emd->getDefFileName(),emd->getDefLine(),emd->getOuterScope(),emd,emd->briefDescription());

	addMarkDownDocBlock(m_output,"detailed",emd->getDefFileName(),emd->getDefLine(),emd->getOuterScope(),emd,emd->documentation());

	m_output.closeHash();
      }
      m_output.closeList();
    }
  }

  const MemberDef *rmd = md->reimplements();
  if (rmd)
    m_output.openHash("reimplements")
      .addFieldQuotedString("name", rmd->name())
      .closeHash();

  MemberList *rbml = md->reimplementedBy();
  if (rbml)
  {
    MemberListIterator mli(*rbml);
    m_output.openList("reimplemented_by");
    for (mli.toFirst();(rmd=mli.current());++mli)
      m_output.openHash()
	.addFieldQuotedString("name", rmd->name())
	.closeHash();
    m_output.closeList();
  }
  
  m_output.closeHash();
}

void MarkDownGenerator::generateMarkDownSection(const Definition *d,
					      MemberList *ml,const char *name,const char *header)
{
  if (ml==0) return; // empty list

  m_output.openHash(name);

  if (header)
    m_output.addFieldQuotedString("header", header);
  
  m_output.openList("members");
  MemberListIterator mli(*ml);
  const MemberDef *md;
  for (mli.toFirst();(md=mli.current());++mli)
  {
    generateMarkDownForMember(md,d);
  }
  m_output.closeList()
    .closeHash();
}

void MarkDownGenerator::addListOfAllMembers(const ClassDef *cd)
{
  m_output.openList("all_members");
  if (cd->memberNameInfoSDict())
  {
    MemberNameInfoSDict::Iterator mnii(*cd->memberNameInfoSDict());
    MemberNameInfo *mni;
    for (mnii.toFirst();(mni=mnii.current());++mnii)
    {
      MemberNameInfoIterator mii(*mni);
      MemberInfo *mi;
      for (mii.toFirst();(mi=mii.current());++mii)
      {
        const MemberDef *md=mi->memberDef;
        const ClassDef  *cd=md->getClassDef();
        const Definition *d=md->getGroupDef();
        if (d==0) d = cd;

        m_output.openHash()
          .addFieldQuotedString("name", md->name())
          .addFieldQuotedString("virtualness", getVirtualnessName(md->virtualness()))
          .addFieldQuotedString("protection", getProtectionName(mi->prot));

        if (!mi->ambiguityResolutionScope.isEmpty())
          m_output.addFieldQuotedString("ambiguity_scope", mi->ambiguityResolutionScope);

        m_output.addFieldQuotedString("scope", cd->name())
          .closeHash();
      }
    }
  }
  m_output.closeList();
}

void MarkDownGenerator::generateMarkDownForClass(const ClassDef *cd)
{
  // + brief description
  // + detailed description
  // + template argument list(s)
  // - include file
  // + member groups
  // + inheritance diagram
  // + list of direct super classes
  // + list of direct sub classes
  // + list of inner classes
  // + collaboration diagram
  // + list of all members
  // + user defined member sections
  // + standard member sections
  // + detailed member documentation
  // - examples using the class
  
  if (cd->isReference())        return; // skip external references.
  if (cd->name().find('@')!=-1) return; // skip anonymous compounds.
  if (cd->templateMaster()!=0)  return; // skip generated template instances.

  m_output.openHash()
    .addFieldQuotedString("name", cd->name());
  
  if (cd->baseClasses())
  {
    m_output.openList("base");
    BaseClassListIterator bcli(*cd->baseClasses());
    BaseClassDef *bcd;
    for (bcli.toFirst();(bcd=bcli.current());++bcli)
      m_output.openHash()
	.addFieldQuotedString("name", bcd->classDef->displayName())
	.addFieldQuotedString("virtualness", getVirtualnessName(bcd->virt))
	.addFieldQuotedString("protection", getProtectionName(bcd->prot))
	.closeHash();
    m_output.closeList();
  }

  if (cd->subClasses())
  {
    m_output.openList("derived");
    BaseClassListIterator bcli(*cd->subClasses());
    BaseClassDef *bcd;
    for (bcli.toFirst();(bcd=bcli.current());++bcli)
      m_output.openHash()
	.addFieldQuotedString("name", bcd->classDef->displayName())
	.addFieldQuotedString("virtualness", getVirtualnessName(bcd->virt))
	.addFieldQuotedString("protection", getProtectionName(bcd->prot))
	.closeHash();
    m_output.closeList();
  }

  ClassSDict *cl = cd->getClassSDict();
  if (cl)
  {
    m_output.openList("inner");
    ClassSDict::Iterator cli(*cl);
    const ClassDef *cd;
    for (cli.toFirst();(cd=cli.current());++cli)
      m_output.openHash()
	.addFieldQuotedString("name", cd->name())
	.closeHash();
    m_output.closeList();
  }

  IncludeInfo *ii=cd->includeInfo();
  if (ii)
  {
    QCString nm = ii->includeName;
    if (nm.isEmpty() && ii->fileDef) nm = ii->fileDef->docName();
    if (!nm.isEmpty())
    {
      m_output.openHash("includes");
#if 0
      if (ii->fileDef && !ii->fileDef->isReference()) // TODO: support external references
        t << " id=\"" << ii->fileDef->getOutputFileBase() << "\"";
#endif
      m_output.addFieldBoolean("local", ii->local)
	.addFieldQuotedString("name", nm)
	.closeHash();
    }
  }

  addTemplateList(cd,m_output);
  addListOfAllMembers(cd);
  if (cd->getMemberGroupSDict())
  {
    MemberGroupSDict::Iterator mgli(*cd->getMemberGroupSDict());
    MemberGroup *mg;
    for (;(mg=mgli.current());++mgli)
      generateMarkDownSection(cd,mg->members(),"user_defined",mg->header());
  }

  generateMarkDownSection(cd,cd->getMemberList(MemberListType_pubTypes),"public_typedefs");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_pubMethods),"public_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_pubAttribs),"public_members");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_pubSlots),"public_slots");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_signals),"signals");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_dcopMethods),"dcop_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_properties),"properties");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_pubStaticMethods),"public_static_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_pubStaticAttribs),"public_static_members");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_proTypes),"protected_typedefs");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_proMethods),"protected_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_proAttribs),"protected_members");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_proSlots),"protected_slots");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_proStaticMethods),"protected_static_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_proStaticAttribs),"protected_static_members");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_priTypes),"private_typedefs");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_priMethods),"private_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_priAttribs),"private_members");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_priSlots),"private_slots");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_priStaticMethods),"private_static_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_priStaticAttribs),"private_static_members");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_friends),"friend_methods");
  generateMarkDownSection(cd,cd->getMemberList(MemberListType_related),"related_methods");

  addMarkDownDocBlock(m_output,"brief",cd->getDefFileName(),cd->getDefLine(),cd,0,cd->briefDescription());
  addMarkDownDocBlock(m_output,"detailed",cd->getDefFileName(),cd->getDefLine(),cd,0,cd->documentation());

#if 0
  DotClassGraph inheritanceGraph(cd,DotClassGraph::Inheritance);
  if (!inheritanceGraph.isTrivial())
  {
    t << "    <inheritancegraph>" << endl;
    inheritanceGraph.writeMarkDown(t);
    t << "    </inheritancegraph>" << endl;
  }
  DotClassGraph collaborationGraph(cd,DotClassGraph::Implementation);
  if (!collaborationGraph.isTrivial())
  {
    t << "    <collaborationgraph>" << endl;
    collaborationGraph.writeMarkDown(t);
    t << "    </collaborationgraph>" << endl;
  }
  t << "    <location file=\"" 
    << cd->getDefFileName() << "\" line=\"" 
    << cd->getDefLine() << "\"";
    if (cd->getStartBodyLine()!=-1)
    {
      t << " bodystart=\"" << cd->getStartBodyLine() << "\" bodyend=\"" 
        << cd->getEndBodyLine() << "\"";
    }
  t << "/>" << endl;
#endif

  m_output.closeHash();
}

void MarkDownGenerator::generateMarkDownForNamespace(const NamespaceDef *nd)
{
  // + contained class definitions
  // + contained namespace definitions
  // + member groups
  // + normal members
  // + brief desc
  // + detailed desc
  // + location
  // - files containing (parts of) the namespace definition

  if (nd->isReference()) return; // skip external references

  m_output.openHash()
    .addFieldQuotedString("name", nd->name());
  
  ClassSDict *cl = nd->getClassSDict();
  if (cl)
  {
    m_output.openList("classes");
    ClassSDict::Iterator cli(*cl);
    const ClassDef *cd;
    for (cli.toFirst();(cd=cli.current());++cli)
      m_output.openHash()
	.addFieldQuotedString("name", cd->name())
	.closeHash();
    m_output.closeList();
  }

  const NamespaceSDict *nl = nd->getNamespaceSDict();
  if (nl)
  {
    m_output.openList("namespaces");
    NamespaceSDict::Iterator nli(*nl);
    const NamespaceDef *nd;
    for (nli.toFirst();(nd=nli.current());++nli)
      m_output.openHash()
	.addFieldQuotedString("name", nd->name())
	.closeHash();
    m_output.closeList();
  }

  if (nd->getMemberGroupSDict())
  {
    MemberGroupSDict::Iterator mgli(*nd->getMemberGroupSDict());
    const MemberGroup *mg;
    for (;(mg=mgli.current());++mgli)
      generateMarkDownSection(nd,mg->members(),"user-defined",mg->header());
  }

  generateMarkDownSection(nd,nd->getMemberList(MemberListType_decDefineMembers),"defines");
  generateMarkDownSection(nd,nd->getMemberList(MemberListType_decProtoMembers),"prototypes");
  generateMarkDownSection(nd,nd->getMemberList(MemberListType_decTypedefMembers),"typedefs");
  generateMarkDownSection(nd,nd->getMemberList(MemberListType_decEnumMembers),"enums");
  generateMarkDownSection(nd,nd->getMemberList(MemberListType_decFuncMembers),"functions");
  generateMarkDownSection(nd,nd->getMemberList(MemberListType_decVarMembers),"variables");

  addMarkDownDocBlock(m_output,"brief",nd->getDefFileName(),nd->getDefLine(),0,0,nd->briefDescription());
  addMarkDownDocBlock(m_output,"detailed",nd->getDefFileName(),nd->getDefLine(),0,0,nd->documentation());

  m_output.closeHash();
}

void MarkDownGenerator::generateMarkDownForFile(const FileDef *fd)
{
  // + includes files
  // + includedby files
  // - include graph
  // - included by graph
  // - contained class definitions
  // - contained namespace definitions
  // - member groups
  // + normal members
  // + brief desc
  // + detailed desc
  // - source code
  // - location
  // - number of lines
 
  if (fd->isReference()) return;

  m_output.openHash()
    .addFieldQuotedString("name", fd->name());
  
  IncludeInfo *inc;
  m_output.openList("includes");
  if (fd->includeFileList())
  {
    QListIterator<IncludeInfo> ili1(*fd->includeFileList());
    for (ili1.toFirst();(inc=ili1.current());++ili1)
    {
      m_output.openHash()
        .addFieldQuotedString("name", inc->includeName);
      if (inc->fileDef && !inc->fileDef->isReference())
      {
        m_output.addFieldQuotedString("ref", inc->fileDef->getOutputFileBase());
      }
      m_output.closeHash();
    }
  }
  m_output.closeList();
  
  m_output.openList("included_by");
  if (fd->includedByFileList())
  {
    QListIterator<IncludeInfo> ili2(*fd->includedByFileList());
    for (ili2.toFirst();(inc=ili2.current());++ili2)
    {
      m_output.openHash()
        .addFieldQuotedString("name", inc->includeName);
      if (inc->fileDef && !inc->fileDef->isReference())
      {
        m_output.addFieldQuotedString("ref", inc->fileDef->getOutputFileBase());
      }
      m_output.closeHash();
    }
  }
  m_output.closeList();
  
  generateMarkDownSection(fd,fd->getMemberList(MemberListType_decDefineMembers),"defines");
  generateMarkDownSection(fd,fd->getMemberList(MemberListType_decProtoMembers),"prototypes");
  generateMarkDownSection(fd,fd->getMemberList(MemberListType_decTypedefMembers),"typedefs");
  generateMarkDownSection(fd,fd->getMemberList(MemberListType_decEnumMembers),"enums");
  generateMarkDownSection(fd,fd->getMemberList(MemberListType_decFuncMembers),"functions");
  generateMarkDownSection(fd,fd->getMemberList(MemberListType_decVarMembers),"variables");

  addMarkDownDocBlock(m_output,"brief",fd->getDefFileName(),fd->getDefLine(),0,0,fd->briefDescription());
  addMarkDownDocBlock(m_output,"detailed",fd->getDefFileName(),fd->getDefLine(),0,0,fd->documentation());

  m_output.closeHash();
}

void MarkDownGenerator::generateMarkDownForGroup(const GroupDef *gd)
{
  // + members
  // + member groups
  // + files
  // + classes
  // + namespaces
  // - packages
  // + pages
  // + child groups
  // - examples
  // + brief description
  // + detailed description

  if (gd->isReference()) return; // skip external references

  m_output.openHash()
    .addFieldQuotedString("name", gd->name())
    .addFieldQuotedString("title", gd->groupTitle());

  FileList *fl = gd->getFiles();
  if (fl)
  {
    m_output.openList("files");
    QListIterator<FileDef> fli(*fl);
    const FileDef *fd;
    for (fli.toFirst();(fd=fli.current());++fli)
      m_output.openHash()
	.addFieldQuotedString("name", fd->name())
	.closeHash();
    m_output.closeList();
  }

  ClassSDict *cl = gd->getClasses();
  if (cl)
  {
    m_output.openList("classes");
    ClassSDict::Iterator cli(*cl);
    const ClassDef *cd;
    for (cli.toFirst();(cd=cli.current());++cli)
      m_output.openHash()
	.addFieldQuotedString("name", cd->name())
	.closeHash();
    m_output.closeList();
  }

  NamespaceSDict *nl = gd->getNamespaces();
  if (nl)
  {
    m_output.openList("namespaces");
    NamespaceSDict::Iterator nli(*nl);
    const NamespaceDef *nd;
    for (nli.toFirst();(nd=nli.current());++nli)
      m_output.openHash()
	.addFieldQuotedString("name", nd->name())
	.closeHash();
    m_output.closeList();
  }

  PageSDict *pl = gd->getPages();
  if (pl)
  {
    m_output.openList("pages");
    PageSDict::Iterator pli(*pl);
    PageDef *pd;
    for (pli.toFirst();(pd=pli.current());++pli)
      m_output.openHash()
	.addFieldQuotedString("title", pd->title())
	.closeHash();
    m_output.closeList();
  }

  GroupList *gl = gd->getSubGroups();
  if (gl)
  {
    m_output.openList("groups");
    GroupListIterator gli(*gl);
    const GroupDef *sgd;
    for (gli.toFirst();(sgd=gli.current());++gli)
      m_output.openHash()
	.addFieldQuotedString("title", sgd->groupTitle())
	.closeHash();
    m_output.closeList();
  }

  if (gd->getMemberGroupSDict())
  {
    MemberGroupSDict::Iterator mgli(*gd->getMemberGroupSDict());
    MemberGroup *mg;
    for (;(mg=mgli.current());++mgli)
      generateMarkDownSection(gd,mg->members(),"user-defined",mg->header());
  }

  generateMarkDownSection(gd,gd->getMemberList(MemberListType_decDefineMembers),"defines");
  generateMarkDownSection(gd,gd->getMemberList(MemberListType_decProtoMembers),"prototypes");
  generateMarkDownSection(gd,gd->getMemberList(MemberListType_decTypedefMembers),"typedefs");
  generateMarkDownSection(gd,gd->getMemberList(MemberListType_decEnumMembers),"enums");
  generateMarkDownSection(gd,gd->getMemberList(MemberListType_decFuncMembers),"functions");
  generateMarkDownSection(gd,gd->getMemberList(MemberListType_decVarMembers),"variables");

  addMarkDownDocBlock(m_output,"brief",gd->getDefFileName(),gd->getDefLine(),0,0,gd->briefDescription());
  addMarkDownDocBlock(m_output,"detailed",gd->getDefFileName(),gd->getDefLine(),0,0,gd->documentation());

  m_output.closeHash();
}

void MarkDownGenerator::generateMarkDownForPage(PageDef *pd)
{
  // + name
  // + title
  // + documentation

  if (pd->isReference()) return;

  m_output.openHash()
    .addFieldQuotedString("name", pd->name());
    
  SectionInfo *si = Doxygen::sectionDict->find(pd->name());
  if (si)
    m_output.addFieldQuotedString("title4", filterTitle(si->title));

  addMarkDownDocBlock(m_output,"detailed",pd->docFile(),pd->docLine(),0,0,pd->documentation());
  m_output.closeHash();
}

bool MarkDownGenerator::generateMarkDownOutput()
{
  QFile outputFile;
  if (!createOutputFile(outputFile, pathDoxyDocsPM))
    return false;
  
  FTextStream outputTextStream(&outputFile);
  MarkDownOutputStream outputStream(&outputTextStream);
  m_output.setMarkDownOutputStream(&outputStream);
  m_output.add("$doxydocs=").openHash();
  
  m_output.openList("classes");
  ClassSDict::Iterator cli(*Doxygen::classSDict);
  const ClassDef *cd;
  for (cli.toFirst();(cd=cli.current());++cli)
    generateMarkDownForClass(cd);
  m_output.closeList();

  m_output.openList("namespaces");
  NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
  const NamespaceDef *nd;
  for (nli.toFirst();(nd=nli.current());++nli)
    generateMarkDownForNamespace(nd);
  m_output.closeList();

  m_output.openList("files");
  FileNameListIterator fnli(*Doxygen::inputNameList);
  FileName *fn;
  for (;(fn=fnli.current());++fnli)
  {
    FileNameIterator fni(*fn);
    const FileDef *fd;
    for (;(fd=fni.current());++fni)
      generateMarkDownForFile(fd);
  }
  m_output.closeList();

  m_output.openList("groups");
  GroupSDict::Iterator gli(*Doxygen::groupSDict);
  const GroupDef *gd;
  for (;(gd=gli.current());++gli)
  {
    generateMarkDownForGroup(gd);
  }
  m_output.closeList();

  m_output.openList("pages");
  PageSDict::Iterator pdi(*Doxygen::pageSDict);
  PageDef *pd=0;
  for (pdi.toFirst();(pd=pdi.current());++pdi)
  {
    generateMarkDownForPage(pd);
  }
  if (Doxygen::mainPage)
  {
    generateMarkDownForPage(Doxygen::mainPage);
  }
  m_output.closeList();

  m_output.closeHash().add(";\n1;\n");
  return true;
}

bool MarkDownGenerator::createOutputFile(QFile &f, const char *s)
{
  f.setName(s);
  if (!f.open(IO_WriteOnly))
  {
    err("Cannot open file %s for writing!\n", s);
    return false;
  }
  return true;
}

bool MarkDownGenerator::createOutputDir(QDir &markDownDir)
{
  QCString outputDirectory = Config_getString(OUTPUT_DIRECTORY);
  if (outputDirectory.isEmpty())
  {
    outputDirectory=QDir::currentDirPath().utf8();
  }
  else
  {
    QDir dir(outputDirectory);
    if (!dir.exists())
    {
      dir.setPath(QDir::currentDirPath());
      if (!dir.mkdir(outputDirectory))
      {
	err("tag OUTPUT_DIRECTORY: Output directory '%s' does not "
	    "exist and cannot be created\n",outputDirectory.data());
	exit(1);
      }
      else
      {
	msg("Notice: Output directory '%s' does not exist. "
	    "I have created it for you.\n", outputDirectory.data());
      }
      dir.cd(outputDirectory);
    }
    outputDirectory=dir.absPath().utf8();
  }

  QDir dir(outputDirectory);
  if (!dir.exists())
  {
    dir.setPath(QDir::currentDirPath());
    if (!dir.mkdir(outputDirectory))
    {
      err("Cannot create directory %s\n",outputDirectory.data());
      return false;
    }
  }
 
  markDownDir.setPath(outputDirectory+"/perlmod");
  if (!markDownDir.exists() && !markDownDir.mkdir(outputDirectory+"/perlmod"))
  {
    err("Could not create perlmod directory in %s\n",outputDirectory.data());
    return false;
  }
  return true;
}

bool PerlModGenerator::generateMakefile()
{
  QFile makefile;
  if (!createOutputFile(makefile, pathMakefile))
    return false;

  bool perlmodLatex = Config_getBool(PERLMOD_LATEX);
  QCString prefix = Config_getString(PERLMOD_MAKEVAR_PREFIX);

  FTextStream makefileStream(&makefile);
  makefileStream <<
    ".PHONY: default clean" << (perlmodLatex ? " pdf" : "") << "\n"
    "default: " << (perlmodLatex ? "pdf" : "clean") << "\n"
    "\n"
    "include " << pathDoxyRules << "\n"
    "\n"
    "clean: clean-perlmod\n";

  if (perlmodLatex) {
    makefileStream <<
      "pdf: $(" << prefix << "DOXYLATEX_PDF)\n"
      "dvi: $(" << prefix << "DOXYLATEX_DVI)\n";
  }

  return true;
}

void MarkDownGenerator::generate()
{
  // + classes
  // + namespaces
  // + files
  // - packages
  // + groups
  // + related pages
  // - examples

  QDir markDownDir;
  if (!createOutputDir(markDownDir))
    return;

  bool perlmodLatex = Config_getBool(PERLMOD_LATEX);

  QCString markDownAbsPath = markDownDir.absPath().utf8();
  pathDoxyDocsPM = markDownAbsPath + "/DoxyDocs.pm";
  pathDoxyStructurePM = markDownAbsPath + "/DoxyStructure.pm";
  pathMakefile = markDownAbsPath + "/Makefile";
  pathDoxyRules = markDownAbsPath + "/doxyrules.make";

  if (perlmodLatex) {
    pathDoxyStructureTex = markDownAbsPath + "/doxystructure.tex";
    pathDoxyFormatTex = markDownAbsPath + "/doxyformat.tex";
    pathDoxyLatexTex = markDownAbsPath + "/doxylatex.tex";
    pathDoxyLatexDVI = markDownAbsPath + "/doxylatex.dvi";
    pathDoxyLatexPDF = markDownAbsPath + "/doxylatex.pdf";
    pathDoxyDocsTex = markDownAbsPath + "/doxydocs.tex";
    pathDoxyLatexPL = markDownAbsPath + "/doxylatex.pl";
    pathDoxyLatexStructurePL = markDownAbsPath + "/doxylatex-structure.pl";
  }

  if (!(generateMarkDownOutput()
	&& generateDoxyStructurePM()
	&& generateMakefile()
	&& generateDoxyRules()))
    return;

  if (perlmodLatex) {
    if (!(generateDoxyLatexStructurePL()
	  && generateDoxyLatexPL()
	  && generateDoxyLatexTex()
	  && generateDoxyFormatTex()))
      return;
  }
}

void generateMarkDown()
{
  MarkDownGenerator pmg(Config_getBool(PERLMOD_PRETTY));
  pmg.generate();
}

