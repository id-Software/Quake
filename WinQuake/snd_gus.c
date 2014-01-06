/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//=============================================================================
// Routines for GUS support in QUAKE
//
// Author(s): Jayeson Lee-Steere
//=============================================================================

#include "quakedef.h"
#include "dosisms.h"

//=============================================================================
// Author(s): Jayeson Lee-Steere

#define INI_STRING_SIZE 0x100

FILE *ini_fopen(const char *filename, const char *modes);
int ini_fclose(FILE *f);
void ini_fgets(FILE *f, const char *section, const char *field, char *s);

// Routines for reading from .INI files
// The read routines are fairly efficient.
//
// Author(s): Jayeson Lee-Steere

#define MAX_SECTION_WIDTH 20
#define MAX_FIELD_WIDTH 20

#define NUM_SECTION_BUFFERS 10
#define NUM_FIELD_BUFFERS 20

struct section_buffer
{
   long offset;
   char name[MAX_SECTION_WIDTH+1];
};

struct field_buffer
{
   long offset;
   int  section;
   char name[MAX_FIELD_WIDTH+1];
};

static FILE *current_file=NULL;
static int   current_section;

static int current_section_buffer=0;
static int current_field_buffer=0;

static struct section_buffer section_buffers[NUM_SECTION_BUFFERS];
static struct field_buffer field_buffers[NUM_FIELD_BUFFERS];
//***************************************************************************
// Internal routines
//***************************************************************************
static char toupper(char c)
{
   if (c>='a' && c<='z')
      c-=('a'-'A');
   return(c);
}

static void reset_buffer(FILE *f)
{
   int i;

   for (i=0;i<NUM_SECTION_BUFFERS;i++)
      section_buffers[i].name[0]=0;
   for (i=0;i<NUM_FIELD_BUFFERS;i++)
      field_buffers[i].name[0]=0;

   current_file=f;
}

// Sees if the current string is section "name" (i.e. ["name"]).
// If "name"=="*", sees if the current string is any section
// (i.e. [....]). Returns 1 if true else 0 if false.
static int is_section(char *s,const char *name)
{
   int wild=0;

   // See if wild search
   if (strcmp("*",name)==0)
      wild=1;

   // Skip leading spaces
   while (s[0]==' ')
      s++;
   // Look for leading "["
   if (s[0]!='[')
      return(0);
   s++;
   // Make sure name matches
   while (s[0]!=']' && s[0]!=13 && s[0]!=10 && s[0]!=0 && name[0]!=0)
   {
      if (!wild)
         if (toupper(s[0])!=toupper(name[0]))
            return(0);
      s++;
      if (!wild)
         name++;
   }
   if (!wild)
      if (name[0]!=0)
         return(0);
   // Skip trailing spaces
   while (s[0]==' ')
      s++;
   // Make sure we have trailing "]"
   if (s[0]!=']')
      return(0);
   return(1);
}

// Sees if the current string is field "name" (i.e. "name"=...).
// If "name"=="*", sees if the current string is any field
// (i.e. ...=...). Returns 1 if true else 0 if false.
static int is_field(char *s,const char *name)
{
   int wild=0;

   // See if wild search
   if (strcmp("*",name)==0)
      wild=1;

   // Skip leading spaces
   while (s[0]==' ')
      s++;

   // Make sure name matches
   while (s[0]!='=' && s[0]!=13 && s[0]!=10 && s[0]!=0 && name[0]!=0)
   {
      if (!wild)
         if (toupper(s[0])!=toupper(name[0]))
            return(0);
      s++;
      if (!wild)
         name++;
   }
   if (!wild)
      if (name[0]!=0)
         return(0);
   // Skip trailing spaces
   while (s[0]==' ')
      s++;
   // Make sure we have an "="
   if (s[0]!='=')
      return(0);

   return(1);
}

// Extracts the section name from a section heading
// e.g. in="[hey man]" gives out="hey man"
static void get_section_name(char *out, char *in)
{
   int i=0;

   // Skip spaces before '['
   while (in[0]==' ')
      in++;
   // Make sure there is a '['
   if (in[0]!='[')
   {
      out[0]=0;
      return;
   }
   // Skip past '['
   in++;
   // Copy string if any to output string.
   while (in[0]!=']' && in[0]!=13 && in[0]!=10 && in[0]!=0)
   {
      if (i<MAX_SECTION_WIDTH)
      {
         out[i]=in[0];
         i++;
      }
      in++;
   }
   // Make sure string was terminated with ']'
   if (in[0]!=']')
   {
      out[0]=0;
      return;
   }
   // Remove trailing spaces
   while (i>0 && out[i-1]==' ')
      i--;
   // Null terminate the output string.
   out[i]=0;
}

// Extracts the field name from a field line
// e.g. in="sooty=life be in it" gives out="sooty"
static void get_field_name(char *out, char *in)
{
   int i=0;

   // Skip leading spaces
   while (in[0]==' ')
      in++;
   // Copy name to output string
   while (in[0]!='=' && in[0]!=13 && in[0]!=10 && in[0]!=0)
   {
      if (i<MAX_FIELD_WIDTH)
      {
         out[i]=in[0];
         i++;
      }
      in++;
   }
   // Make sure we stopped on "="
   if (in[0]!='=')
   {
      out[0]=0;
      return;
   }
   // Remove trailing spaces
   while (i>0 && out[i-1]==' ')
      i--;
   // Null terminate the output string.
   out[i]=0;
}

// Returns the field data from string s.
// e.g. in="wally = golly man" gives out="golly man"
static void get_field_string(char *out, char *in)
{
   int i=0;

   // Find '=' if it exists
   while (in[0]!='=' && in[0]!=13 && in[0]!=10 && in[0]!=0)
      in++;
   // If there is an '=', skip past it.
   if (in[0]=='=')
      in++;
   // Skip any spaces between the '=' and string.
   while (in[0]==' ' || in[0]=='[')
      in++;
   // Copy string, if there is one, to the output string.
   while (in[0]!=13 && in[0]!=10 && in[0]!=0 && i<(INI_STRING_SIZE-1))
   {
      out[i]=in[0];
      in++;
      i++;
   }
   // Null terminate the output string.
   out[i]=0;
}

// Adds a section to the buffer
static int add_section(char *instring, long offset)
{
   int i;
   char section[MAX_SECTION_WIDTH+1];

   // Extract section name
   get_section_name(section,instring);
   // See if section already exists.
   for (i=0;i<NUM_SECTION_BUFFERS;i++)
      if (stricmp(section,section_buffers[i].name)==0)
         return(i);
   // Increment current_section_buffer
   current_section_buffer++;
   if (current_section_buffer>NUM_SECTION_BUFFERS)
      current_section_buffer=0;
   // Delete any field buffers that correspond to this section
   for (i=0;i<NUM_FIELD_BUFFERS;i++)
      if (field_buffers[i].section==current_section_buffer)
         field_buffers[i].name[0]=0;
   // Set buffer information
   strcpy(section_buffers[current_section_buffer].name,section);
   section_buffers[current_section_buffer].offset=offset;
   return(current_section_buffer);
}

// Adds a field to the buffer
static void add_field(char *instring, int section, long offset)
{
   int i;
   char field[MAX_FIELD_WIDTH+1];

   // Extract field name
   get_field_name(field,instring);
   // See if field already exists
   for (i=0;i<NUM_FIELD_BUFFERS;i++)
      if (field_buffers[i].section==section)
         if (stricmp(field_buffers[i].name,field)==0)
            return;
   // Increment current_field_buffer
   current_field_buffer++;
   if (current_field_buffer>NUM_FIELD_BUFFERS)
      current_field_buffer=0;
   // Set buffer information
   strcpy(field_buffers[current_field_buffer].name,field);
   field_buffers[current_field_buffer].section=section;
   field_buffers[current_field_buffer].offset=offset;
}

// Identical to fgets except the string is trucated at the first ';',
// carriage return or line feed.
static char *stripped_fgets(char *s, int n, FILE *f)
{
   int i=0;

   if (fgets(s,n,f)==NULL)
      return(NULL);

   while (s[i]!=';' && s[i]!=13 && s[i]!=10 && s[i]!=0)
      i++;
   s[i]=0;

   return(s);
}

//***************************************************************************
// Externally accessable routines
//***************************************************************************
// Opens an .INI file. Works like fopen
FILE *ini_fopen(const char *filename, const char *modes)
{
   return(fopen(filename,modes));
}

// Closes a .INI file. Works like fclose
int ini_fclose(FILE *f)
{
   if (f==current_file)
      reset_buffer(NULL);
   return(fclose(f));
}

// Puts "field" from "section" from .ini file "f" into "s".
// If "section" does not exist or "field" does not exist in
// section then s="";
void ini_fgets(FILE *f, const char *section, const char *field, char *s)
{
   int i;
   long start_pos,string_start_pos;
   char ts[INI_STRING_SIZE*2];

   if (f!=current_file)
      reset_buffer(f);

   // Default to "Not found"
   s[0]=0;

   // See if section is in buffer
   for (i=0;i<NUM_SECTION_BUFFERS;i++)
      if (strnicmp(section_buffers[i].name,section,MAX_SECTION_WIDTH)==0)
         break;

   // If section is in buffer, seek to it if necessary
   if (i<NUM_SECTION_BUFFERS)
   {
      if (i!=current_section)
      {
         current_section=i;
         fseek(f,section_buffers[i].offset,SEEK_SET);
      }
   }
   // else look through .ini file for it.
   else
   {
      // Make sure we are not at eof or this will cause trouble.
      if (feof(f))
         rewind(f);
      start_pos=ftell(f);
      while (1)
      {
         stripped_fgets(ts,INI_STRING_SIZE*2,f);
         // If it is a section, add it to the section buffer
         if (is_section(ts,"*"))
            current_section=add_section(ts,ftell(f));
         // If it is the section we are looking for, break.
         if (is_section(ts,section))
            break;
         // If we reach the end of the file, rewind to the start.
         if (feof(f))
            rewind(f);
         if (ftell(f)==start_pos)
            return;
      }
   }

   // See if field is in buffer
   for (i=0;i<NUM_FIELD_BUFFERS;i++)
      if (field_buffers[i].section==current_section)
         if (strnicmp(field_buffers[i].name,field,MAX_FIELD_WIDTH)==0)
            break;

   // If field is in buffer, seek to it and read it
   if (i<NUM_FIELD_BUFFERS)
   {
      fseek(f,field_buffers[i].offset,SEEK_SET);
      stripped_fgets(ts,INI_STRING_SIZE*2,f);
      get_field_string(s,ts);
   }
   else
   // else search through section for field.
   {
      // Make sure we do not start at eof or this will cause problems.
      if (feof(f))
         fseek(f,section_buffers[current_section].offset,SEEK_SET);
      start_pos=ftell(f);
      while (1)
      {
         string_start_pos=ftell(f);
         stripped_fgets(ts,INI_STRING_SIZE*2,f);
         // If it is a field, add it to the buffer
         if (is_field(ts,"*"))
            add_field(ts,current_section,string_start_pos);
         // If it is the field we are looking for, save it
         if (is_field(ts,field))
         {
            get_field_string(s,ts);
            break;
         }
         // If we reach the end of the section, start over
         if (feof(f) || is_section(ts,"*"))
            fseek(f,section_buffers[current_section].offset,SEEK_SET);
         if (ftell(f)==start_pos)
            return;
      }
   }
}

//=============================================================================

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long

#define BUFFER_SIZE 4096

#define CODEC_ADC_INPUT_CONTROL_LEFT	0x00
#define CODEC_ADC_INPUT_CONTROL_RIGHT	0x01
#define CODEC_AUX1_INPUT_CONTROL_LEFT	0x02
#define CODEC_AUX1_INPUT_CONTROL_RIGHT	0x03
#define CODEC_AUX2_INPUT_CONTROL_LEFT	0x04
#define CODEC_AUX2_INPUT_CONTROL_RIGHT	0x05
#define CODEC_DAC_OUTPUT_CONTROL_LEFT	0x06
#define CODEC_DAC_OUTPUT_CONTROL_RIGHT	0x07
#define CODEC_FS_FORMAT			0x08
#define CODEC_INTERFACE_CONFIG		0x09
#define CODEC_PIN_CONTROL		0x0A
#define CODEC_ERROR_STATUS_AND_INIT	0x0B
#define CODEC_MODE_AND_ID		0x0C
#define CODEC_LOOPBACK_CONTROL		0x0D
#define CODEC_PLAYBACK_UPPER_BASE_COUNT	0x0E
#define CODEC_PLAYBACK_LOWER_BASE_COUNT	0x0F

#define SET_CONTROL			0x00
#define SET_FREQUENCY			0x01
#define SET_START_HIGH			0x02
#define SET_START_LOW			0x03
#define SET_END_HIGH			0x04
#define SET_END_LOW			0x05
#define SET_VOLUME_RATE			0x06
#define SET_VOLUME_START		0x07
#define SET_VOLUME_END			0x08
#define SET_CURR_VOLUME			0x09
#define SET_VOLUME			0x09
#define SET_ACC_HIGH			0x0A
#define SET_ACC_LOW			0x0B
#define SET_BALANCE			0x0C
#define SET_VOLUME_CONTROL		0x0D
#define SET_VOICES			0x0E

#define DMA_CONTROL			0x41
#define SET_DMA_ADDRESS			0x42
#define SET_DRAM_LOW			0x43
#define SET_DRAM_HIGH			0x44
#define ADLIB_CONTROL			0x45
#define ADLIB_TIMER1			0x46
#define ADLIB_TIMER2			0x47
#define SET_RECORD_RATE			0x48
#define RECORD_CONTROL			0x49
#define SET_JOYSTICK			0x4B
#define MASTER_RESET			0x4C

#define GET_CONTROL			0x80
#define GET_FREQUENCY			0x81
#define GET_START_HIGH			0x82
#define GET_START_LOW			0x83
#define GET_END_HIGH			0x84
#define GET_END_LOW			0x85
#define GET_VOLUME_RATE			0x86
#define GET_VOLUME_START		0x87
#define GET_VOLUME_END			0x88
#define GET_VOLUME			0x89
#define GET_ACC_HIGH			0x8A
#define GET_ACC_LOW			0x8B
#define GET_BALANCE			0x8C
#define GET_VOLUME_CONTROL		0x8D
#define GET_VOICES			0x8E
#define GET_IRQV                        0x8F

struct CodecRateStruct
{
   WORD Rate;
   BYTE FSVal;
};

struct Gf1RateStruct
{
   WORD Rate;
   BYTE Voices;
};

//=============================================================================
// Reference variables in SND_DOS.C
//=============================================================================
extern short *dma_buffer;

//=============================================================================
// GUS-only variables
//=============================================================================
static BYTE HaveCodec=0;

static WORD CodecRegisterSelect;
static WORD CodecData;
static WORD CodecStatus;
static WORD Gf1TimerControl;
static WORD Gf1PageRegister;
static WORD Gf1RegisterSelect;
static WORD Gf1DataLow;
static WORD Gf1DataHigh;

static BYTE DmaChannel;

static BYTE PageRegs[] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };
static BYTE AddrRegs[] = { 0, 2, 4, 6, 0xc0, 0xc4, 0xc8, 0xcc };
static BYTE CountRegs[] = { 1, 3, 5, 7, 0xc2, 0xc6, 0xca, 0xce };

static WORD AddrReg;
static WORD CountReg;
static WORD ModeReg;
static WORD DisableReg;
static WORD ClearReg;

static struct CodecRateStruct CodecRates[]=
{
   { 5512,0x01},
   { 6620,0x0F},
   { 8000,0x00},
   { 9600,0x0E},
   {11025,0x03},
   {16000,0x02},
   {18900,0x05},
   {22050,0x07},
   {27420,0x04},
   {32000,0x06},
   {33075,0x0D},
   {37800,0x09},
   {44100,0x0B},
   {48000,0x0C},
   {    0,0x00} // End marker
};

static struct Gf1RateStruct Gf1Rates[]=
{
   {19293,32},
   {19916,31},
   {20580,30},
   {21289,29},
   {22050,28},
   {22866,27},
   {23746,26},
   {24696,25},
   {25725,24},
   {26843,23},
   {28063,22},
   {29400,21},
   {30870,20},
   {32494,19},
   {34300,18},
   {36317,17},
   {38587,16},
   {41160,15},
   {44100,14},
   {0,0}
};

//=============================================================================
// Basic GF1 functions
//=============================================================================
void SetGf18(BYTE reg,BYTE data)
{
   dos_outportb(Gf1RegisterSelect,reg);
   dos_outportb(Gf1DataHigh,data);
}

void SetGf116(BYTE reg,WORD data)
{
   dos_outportb(Gf1RegisterSelect,reg);
   dos_outportw(Gf1DataLow,data);
}

BYTE GetGf18(BYTE reg)
{
   dos_outportb(Gf1RegisterSelect,reg);
   return(dos_inportb(Gf1DataHigh));
}

WORD GetGf116(BYTE reg)
{
   dos_outportb(Gf1RegisterSelect,reg);
   return(dos_inportw(Gf1DataLow));
}

void Gf1Delay(void)
{
   int i;

   for (i=0;i<27;i++)
      dos_inportb(Gf1TimerControl);
}

DWORD ConvertTo16(DWORD Address)
{
   return( ((Address>>1) & 0x0001FFFF) | (Address & 0x000C0000L) );
}

void ClearGf1Ints(void)
{
   int i;

   SetGf18(DMA_CONTROL,0x00);
   SetGf18(ADLIB_CONTROL,0x00);
   SetGf18(RECORD_CONTROL,0x00);
		
   GetGf18(DMA_CONTROL);
   GetGf18(RECORD_CONTROL);
   for (i=0;i<32;i++);
      GetGf18(GET_IRQV);
}


//=============================================================================
// Get Interwave (UltraSound PnP) configuration if any
//=============================================================================
static qboolean GUS_GetIWData(void)
{
   char *Interwave,s[INI_STRING_SIZE];
   FILE *IwFile;
   int  CodecBase,CodecDma,i;

   Interwave=getenv("INTERWAVE");
   if (Interwave==NULL)
      return(false);

   // Open IW.INI
   IwFile=ini_fopen(Interwave,"rt");
   if (IwFile==NULL)
      return(false);

   // Read codec base and codec DMA
   ini_fgets(IwFile,"setup 0","CodecBase",s);
   sscanf(s,"%X",&CodecBase);
   ini_fgets(IwFile,"setup 0","DMA2",s);
   sscanf(s,"%i",&CodecDma);

   ini_fclose(IwFile);

   // Make sure numbers OK
   if (CodecBase==0 || CodecDma==0)
      return(false);

   CodecRegisterSelect=CodecBase;
   CodecData=CodecBase+1;
   CodecStatus=CodecBase+2;
   DmaChannel=CodecDma;

   // Make sure there is a CODEC at the CODEC base

   // Clear any pending IRQs
   dos_inportb(CodecStatus);
   dos_outportb(CodecStatus,0);

   // Wait for 'INIT' bit to clear
   for (i=0;i<0xFFFF;i++)
      if ((dos_inportb(CodecRegisterSelect) & 0x80) == 0)
         break;
   if (i==0xFFFF)
      return(false);

   // Get chip revision - can not be zero
   dos_outportb(CodecRegisterSelect,CODEC_MODE_AND_ID);
   if ((dos_inportb(CodecRegisterSelect) & 0x7F) != CODEC_MODE_AND_ID)
      return(false);
   if ((dos_inportb(CodecData) & 0x0F) == 0)
      return(false);

   HaveCodec=1;
   Con_Printf("Sound Card is UltraSound PnP\n");
   return(true);
}

//=============================================================================
// Get UltraSound MAX configuration if any
//=============================================================================
static qboolean GUS_GetMAXData(void)
{
   char *Ultrasnd,*Ultra16;
   int  i;
   int  GusBase,Dma1,Dma2,Irq1,Irq2;
   int  CodecBase,CodecDma,CodecIrq,CodecType;
   BYTE MaxVal;

   Ultrasnd=getenv("ULTRASND");
   Ultra16=getenv("ULTRA16");
   if (Ultrasnd==NULL || Ultra16==NULL)
      return(false);

   sscanf(Ultrasnd,"%x,%i,%i,%i,%i",&GusBase,&Dma1,&Dma2,&Irq1,&Irq2);
   sscanf(Ultra16,"%x,%i,%i,%i",&CodecBase,&CodecDma,&CodecIrq,&CodecType);

   if (CodecType==0 && CodecDma!=0)
      DmaChannel=CodecDma & 0x07;
   else
      DmaChannel=Dma2 & 0x07;

   // Make sure there is a GUS at GUS base
   dos_outportb(GusBase+0x08,0x55);
   if (dos_inportb(GusBase+0x0A)!=0x55)
      return(false);
   dos_outportb(GusBase+0x08,0xAA);
   if (dos_inportb(GusBase+0x0A)!=0xAA)
      return(false);

   // Program CODEC control register
   MaxVal=((CodecBase & 0xF0)>>4) | 0x40;
   if (Dma1 > 3)
      MaxVal|=0x10;
   if (Dma2 > 3)
      MaxVal|=0x20;
   dos_outportb(GusBase+0x106,MaxVal);

   CodecRegisterSelect=CodecBase;
   CodecData=CodecBase+1;
   CodecStatus=CodecBase+2;

   // Make sure there is a CODEC at the CODEC base

   // Clear any pending IRQs
   dos_inportb(CodecStatus);
   dos_outportb(CodecStatus,0);

   // Wait for 'INIT' bit to clear
   for (i=0;i<0xFFFF;i++)
      if ((dos_inportb(CodecRegisterSelect) & 0x80) == 0)
         break;
   if (i==0xFFFF)
      return(false);

   // Get chip revision - can not be zero
   dos_outportb(CodecRegisterSelect,CODEC_MODE_AND_ID);
   if ((dos_inportb(CodecRegisterSelect) & 0x7F) != CODEC_MODE_AND_ID)
      return(false);
   if ((dos_inportb(CodecData) & 0x0F) == 0)
      return(false);

   HaveCodec=1;
   Con_Printf("Sound Card is UltraSound MAX\n");
   return(true);
}

//=============================================================================
// Get regular UltraSound configuration if any
//=============================================================================
static qboolean GUS_GetGUSData(void)
{
   char *Ultrasnd;
   int  GusBase,Dma1,Dma2,Irq1,Irq2,i;

   Ultrasnd=getenv("ULTRASND");
   if (Ultrasnd==NULL)
      return(false);

   sscanf(Ultrasnd,"%x,%i,%i,%i,%i",&GusBase,&Dma1,&Dma2,&Irq1,&Irq2);

   DmaChannel=Dma1 & 0x07;

   // Make sure there is a GUS at GUS base
   dos_outportb(GusBase+0x08,0x55);
   if (dos_inportb(GusBase+0x0A)!=0x55)
      return(false);
   dos_outportb(GusBase+0x08,0xAA);
   if (dos_inportb(GusBase+0x0A)!=0xAA)
      return(false);

   Gf1TimerControl   = GusBase+0x008;
   Gf1PageRegister   = GusBase+0x102;
   Gf1RegisterSelect = GusBase+0x103;
   Gf1DataLow        = GusBase+0x104;
   Gf1DataHigh       = GusBase+0x105;

   // Reset the GUS
   SetGf18(MASTER_RESET,0x00);
   Gf1Delay();
   Gf1Delay();
   SetGf18(MASTER_RESET,0x01);
   Gf1Delay();
   Gf1Delay();

   // Set to max (32) voices
   SetGf18(SET_VOICES,0xDF);

   // Clear any pending IRQ's
   ClearGf1Ints();

   // Set all registers to known values
   for (i=0;i<32;i++)
   {
      dos_outportb(Gf1PageRegister,i);
      SetGf18(SET_CONTROL,0x03);
      SetGf18(SET_VOLUME_CONTROL,0x03);
      Gf1Delay();
      SetGf18(SET_CONTROL,0x03);
      SetGf18(SET_VOLUME_CONTROL,0x03);
      SetGf116(SET_START_HIGH,0);
      SetGf116(SET_START_LOW,0);
      SetGf116(SET_END_HIGH,0);
      SetGf116(SET_END_LOW,0);
      SetGf116(SET_ACC_HIGH,0);
      SetGf116(SET_ACC_LOW,0);
      SetGf18(SET_VOLUME_RATE,63);
      SetGf18(SET_VOLUME_START,5);
      SetGf18(SET_VOLUME_END,251);
      SetGf116(SET_VOLUME,5<<8);
   }

   // Clear any pending IRQ's
   ClearGf1Ints();

   // Enable DAC etc.
   SetGf18(MASTER_RESET,0x07);

   // Enable line output so we can hear something
   dos_outportb(GusBase,0x08);

   HaveCodec=0;
   Con_Printf("Sound Card is UltraSound\n");
   return(true);
}


//=============================================================================
// Programs the DMA controller to start DMAing in Auto-init mode
//=============================================================================
static void GUS_StartDMA(BYTE DmaChannel,short *dma_buffer,int count)
{
   int mode;
   int RealAddr;

   RealAddr = ptr2real(dma_buffer);

   if (DmaChannel <= 3)
   {
      ModeReg = 0x0B;
      DisableReg = 0x0A;
      ClearReg = 0x0E;
   }
   else
   {
      ModeReg = 0xD6;
      DisableReg = 0xD4;
      ClearReg = 0xDC;
   }
   CountReg=CountRegs[DmaChannel];
   AddrReg=AddrRegs[DmaChannel];

   dos_outportb(DisableReg, DmaChannel | 4);	// disable channel

   // set mode- see "undocumented pc", p.876
   mode = (1<<6)	        // single-cycle
          +(0<<5)	        // address increment
	  +(1<<4)	        // auto-init dma
	  +(2<<2)	        // read
	  +(DmaChannel & 0x03);	// channel #
   dos_outportb(ModeReg, mode);

   // set page
   dos_outportb(PageRegs[DmaChannel], RealAddr >> 16);

   if (DmaChannel <= 3)
   {	// address is in bytes
      dos_outportb(0x0C, 0);		// prepare to send 16-bit value
      dos_outportb(AddrReg, RealAddr & 0xff);
      dos_outportb(AddrReg, (RealAddr>>8) & 0xff);

      dos_outportb(0x0C, 0);		// prepare to send 16-bit value
      dos_outportb(CountReg, (count-1) & 0xff);
      dos_outportb(CountReg, (count-1) >> 8);
   }
   else
   {	// address is in words
      dos_outportb(0xD8, 0);	        // prepare to send 16-bit value
      dos_outportb(AddrReg, (RealAddr>>1) & 0xff);
      dos_outportb(AddrReg, (RealAddr>>9) & 0xff);

      dos_outportb(0xD8, 0);		// prepare to send 16-bit value
      dos_outportb(CountReg, ((count>>1)-1) & 0xff);
      dos_outportb(CountReg, ((count>>1)-1) >> 8);
   }

   dos_outportb(ClearReg, 0);		// clear write mask
   dos_outportb(DisableReg, DmaChannel & ~4);
}

//=============================================================================
// Starts the CODEC playing
//=============================================================================
static void GUS_StartCODEC(int count,BYTE FSVal)
{
   int i,j;

   // Clear any pending IRQs
   dos_inportb(CodecStatus);
   dos_outportb(CodecStatus,0);

   // Set mode to 2
   dos_outportb(CodecRegisterSelect,CODEC_MODE_AND_ID);
   dos_outportb(CodecData,0xC0);

   // Stop any playback or capture which may be happening
   dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG);
   dos_outportb(CodecData,dos_inportb(CodecData) & 0xFC);

   // Set FS
   dos_outportb(CodecRegisterSelect,CODEC_FS_FORMAT | 0x40);
   dos_outportb(CodecData,FSVal | 0x50); // Or in stereo and 16 bit bits

   // Wait a bit
   for (i=0;i<10;i++)
      dos_inportb(CodecData);

   // Routine 1 to counter CODEC bug - wait for init bit to clear and then a
   // bit longer (i=min loop count, j=timeout
   for (i=0,j=0;i<1000 && j<0x7FFFF;j++)
      if ((dos_inportb(CodecRegisterSelect) & 0x80)==0)
         i++;

   // Routine 2 to counter CODEC bug - this is from Forte's code. For me it
   // does not seem to cure the problem, but is added security
   // Waits till we can modify index register
   for (j=0;j<0x7FFFF;j++)
   {
      dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG | 0x40);
      if (dos_inportb(CodecRegisterSelect)==(CODEC_INTERFACE_CONFIG | 0x40))
         break;
   }

   // Perform ACAL
   dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG | 0x40);
   dos_outportb(CodecData,0x08);

   // Clear MCE bit - this makes ACAL happen
   dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG);

   // Wait for ACAL to finish
   for (j=0;j<0x7FFFF;j++)
   {
      if ((dos_inportb(CodecRegisterSelect) & 0x80) != 0)
         continue;
      dos_outportb(CodecRegisterSelect,CODEC_ERROR_STATUS_AND_INIT);
      if ((dos_inportb(CodecData) & 0x20) == 0)
         break;
   }

   // Clear ACAL bit
   dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG | 0x40);
   dos_outportb(CodecData,0x00);
   dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG);

   // Set some other junk
   dos_outportb(CodecRegisterSelect,CODEC_LOOPBACK_CONTROL);
   dos_outportb(CodecData,0x00);
   dos_outportb(CodecRegisterSelect,CODEC_PIN_CONTROL);
   dos_outportb(CodecData,0x08); // IRQ is disabled in PIN control

   // Set count (it doesn't really matter what value we stuff in here
   dos_outportb(CodecRegisterSelect,CODEC_PLAYBACK_LOWER_BASE_COUNT);
   dos_outportb(CodecData,count & 0xFF);
   dos_outportb(CodecRegisterSelect,CODEC_PLAYBACK_UPPER_BASE_COUNT);
   dos_outportb(CodecData,count >> 8);

   // Start playback
   dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG);
   dos_outportb(CodecData,0x01);
}

//=============================================================================
// Starts the GF1 playing
//=============================================================================
static void GUS_StartGf1(int count,BYTE Voices)
{
   DWORD StartAddressL,EndAddressL,StartAddressR,EndAddressR;

   // Set number of voices to give us the sampling rate we want
   SetGf18(SET_VOICES,0xC0 | (Voices-1));

   // Figure out addresses
   StartAddressL=ConvertTo16(0);
   EndAddressL=ConvertTo16(count-2-2);
   StartAddressR=ConvertTo16(2);
   EndAddressR=ConvertTo16(count-2);

   // Set left voice addresses
   dos_outportb(Gf1PageRegister,0);
   SetGf116(SET_START_LOW,StartAddressL<<9);
   SetGf116(SET_START_HIGH,StartAddressL>>7);
   SetGf116(SET_ACC_LOW,StartAddressL<<9);
   SetGf116(SET_ACC_HIGH,StartAddressL>>7);
   SetGf116(SET_END_LOW,EndAddressL<<9);
   SetGf116(SET_END_HIGH,EndAddressL>>7);
   // Set balance to full left
   SetGf18(SET_BALANCE,0);
   // Set volume to full
   SetGf116(SET_VOLUME,0xFFF0);
   // Set FC to 2 (so we play every second sample)
   SetGf116(SET_FREQUENCY,0x0800);

   // Set right voice addresses
   dos_outportb(Gf1PageRegister,1);
   SetGf116(SET_START_LOW,StartAddressR<<9);
   SetGf116(SET_START_HIGH,StartAddressR>>7);
   SetGf116(SET_ACC_LOW,StartAddressR<<9);
   SetGf116(SET_ACC_HIGH,StartAddressR>>7);
   SetGf116(SET_END_LOW,EndAddressR<<9);
   SetGf116(SET_END_HIGH,EndAddressR>>7);
   // Set balance to full right
   SetGf18(SET_BALANCE,15);
   // Set volume to full
   SetGf116(SET_VOLUME,0xFFF0);
   // Set FC to 2 (so we play every second sample)
   SetGf116(SET_FREQUENCY,0x0800);

   // Start voices
   dos_outportb(Gf1PageRegister,0);
   SetGf18(SET_CONTROL,0x0C);
   dos_outportb(Gf1PageRegister,1);
   SetGf18(SET_CONTROL,0x0C);
   Gf1Delay();
   dos_outportb(Gf1PageRegister,0);
   SetGf18(SET_CONTROL,0x0C);
   dos_outportb(Gf1PageRegister,1);
   SetGf18(SET_CONTROL,0x0C);
}


//=============================================================================
// Figures out what kind of UltraSound we have, if any, and starts it playing
//=============================================================================
qboolean GUS_Init(void)
{
	int rc;
	int RealAddr;
	BYTE FSVal,Voices;
	struct CodecRateStruct *CodecRate;
	struct Gf1RateStruct *Gf1Rate;

	// See what kind of UltraSound we have, if any
	if (GUS_GetIWData()==false)
		if (GUS_GetMAXData()==false)
			if (GUS_GetGUSData()==false)
				return(false);

	shm = &sn;

	if (HaveCodec)
	{
		// do 11khz sampling rate unless command line parameter wants different
		shm->speed = 11025;
		FSVal = 0x03;
		rc = COM_CheckParm("-sspeed");
		if (rc)
		{
			shm->speed = Q_atoi(com_argv[rc+1]);
	
			// Make sure rate not too high
			if (shm->speed>48000)
				shm->speed=48000;
	
			// Adjust speed to match one of the possible CODEC rates
			for (CodecRate=CodecRates;CodecRate->Rate!=0;CodecRate++)
			{
				if (shm->speed <= CodecRate->Rate)
				{
					shm->speed=CodecRate->Rate;
					FSVal=CodecRate->FSVal;
					break;
				}
			}
		}

	
		// Always do 16 bit stereo
		shm->channels = 2;
		shm->samplebits = 16;
	
		// allocate buffer twice the size we need so we can get aligned buffer
		dma_buffer = dos_getmemory(BUFFER_SIZE*2);
		if (dma_buffer==NULL)
		{
			Con_Printf("Couldn't allocate sound dma buffer");
			return false;
		}

		RealAddr = ptr2real(dma_buffer);
		RealAddr = (RealAddr + BUFFER_SIZE) & ~(BUFFER_SIZE-1);
		dma_buffer = (short *) real2ptr(RealAddr);

		// Zero off DMA buffer
		memset(dma_buffer, 0, BUFFER_SIZE);

		shm->soundalive = true;
		shm->splitbuffer = false;

		shm->samplepos = 0;
		shm->submission_chunk = 1;
		shm->buffer = (unsigned char *) dma_buffer;
		shm->samples = BUFFER_SIZE/(shm->samplebits/8);

		GUS_StartDMA(DmaChannel,dma_buffer,BUFFER_SIZE);
		GUS_StartCODEC(BUFFER_SIZE,FSVal);
	}
	else
	{
		// do 19khz sampling rate unless command line parameter wants different
		shm->speed = 19293;
		Voices=32;
		rc = COM_CheckParm("-sspeed");
		if (rc)
		{
			shm->speed = Q_atoi(com_argv[rc+1]);

			// Make sure rate not too high
			if (shm->speed>44100)
				shm->speed=44100;

			// Adjust speed to match one of the possible GF1 rates
			for (Gf1Rate=Gf1Rates;Gf1Rate->Rate!=0;Gf1Rate++)
			{
				if (shm->speed <= Gf1Rate->Rate)
				{
					shm->speed=Gf1Rate->Rate;
					Voices=Gf1Rate->Voices;
					break;
				}
			}
		}

		// Always do 16 bit stereo
		shm->channels = 2;
		shm->samplebits = 16;

		// allocate buffer twice the size we need so we can get aligned buffer
		dma_buffer = dos_getmemory(BUFFER_SIZE*2);
		if (dma_buffer==NULL)
		{
			Con_Printf("Couldn't allocate sound dma buffer");
			return false;
		}

		RealAddr = ptr2real(dma_buffer);
		RealAddr = (RealAddr + BUFFER_SIZE) & ~(BUFFER_SIZE-1);
		dma_buffer = (short *) real2ptr(RealAddr);

		// Zero off DMA buffer
		memset(dma_buffer, 0, BUFFER_SIZE);

		shm->soundalive = true;
		shm->splitbuffer = false;

		shm->samplepos = 0;
		shm->submission_chunk = 1;
		shm->buffer = (unsigned char *) dma_buffer;
		shm->samples = BUFFER_SIZE/(shm->samplebits/8);

		GUS_StartDMA(DmaChannel,dma_buffer,BUFFER_SIZE);
		SetGf116(SET_DMA_ADDRESS,0x0000);
		if (DmaChannel<=3)
			SetGf18(DMA_CONTROL,0x41);
		else
			SetGf18(DMA_CONTROL,0x45);
		GUS_StartGf1(BUFFER_SIZE,Voices);
	}
	return(true);
}

//=============================================================================
// Returns the current playback position
//=============================================================================
int GUS_GetDMAPos(void)
{
   int count;

	if (HaveCodec)
	{
	   // clear 16-bit reg flip-flop
 	  // load the current dma count register
 	  if (DmaChannel < 4)
 	  {
 	     dos_outportb(0x0C, 0);
 	     count = dos_inportb(CountReg);
 	     count += dos_inportb(CountReg) << 8;
 	     if (shm->samplebits == 16)
 	        count /= 2;
 	     count = shm->samples - (count+1);
 	  }
 	  else
 	  {
 	     dos_outportb(0xD8, 0);
 	     count = dos_inportb(CountReg);
 	     count += dos_inportb(CountReg) << 8;
 	     if (shm->samplebits == 8)
 	        count *= 2;
 	     count = shm->samples - (count+1);
 	  }

	}
	else
	{
		// Read current position from GF1
		dos_outportb(Gf1PageRegister,0);
		count=(GetGf116(GET_ACC_HIGH)<<7) & 0xFFFF;
		// See which half of buffer we are in. Note that since this is 16 bit
		// data we are playing, position is in 16 bit samples
		if (GetGf18(DMA_CONTROL) & 0x40)
		{
			GUS_StartDMA(DmaChannel,dma_buffer,BUFFER_SIZE);
			SetGf116(SET_DMA_ADDRESS,0x0000);
			if (DmaChannel<=3)
				SetGf18(DMA_CONTROL,0x41);
			else
				SetGf18(DMA_CONTROL,0x45);
		}
	}

   shm->samplepos = count & (shm->samples-1);
   return(shm->samplepos);
}

//=============================================================================
// Stops the UltraSound playback
//=============================================================================
void GUS_Shutdown (void)
{
	if (HaveCodec)
	{
		// Stop CODEC
		dos_outportb(CodecRegisterSelect,CODEC_INTERFACE_CONFIG);
		dos_outportb(CodecData,0x01);
	}
	else
	{
		// Stop Voices
		dos_outportb(Gf1PageRegister,0);
		SetGf18(SET_CONTROL,0x03);
		dos_outportb(Gf1PageRegister,1);
		SetGf18(SET_CONTROL,0x03);
		Gf1Delay();
		dos_outportb(Gf1PageRegister,0);
		SetGf18(SET_CONTROL,0x03);
		dos_outportb(Gf1PageRegister,1);
		SetGf18(SET_CONTROL,0x03);

		// Stop any DMA
		SetGf18(DMA_CONTROL,0x00);
		GetGf18(DMA_CONTROL);
	}

	dos_outportb(DisableReg, DmaChannel | 4); // disable dma channel
}
