#pragma once


class CustomGCmd{
  protected:
  char Gletter;
  int  GValue;
  public:
  CustomGCmd(char gcletter, int gcValue)
  {
    Gletter=gcletter;
    GValue=gcValue;
  }
  virtual bool match(char gcletter, int gcValue,char* GCLine)
  {
    return (gcletter==Gletter && GValue==gcValue);
  }
  virtual int parse(char* line, uint8_t *char_offset)
  {
    return 0;
  }

  virtual int act()
  {
    return 0;
  }

};

class CustomGCmdSets{
public:
  CustomGCmd **cmdList;
  int listL;

  void set(CustomGCmd **_cmdlist,int _listL)
  {
    cmdList=_cmdlist;
    listL=_listL;
  }



  virtual CustomGCmd *find(char Gletter,int  GValue,char* GCLine)
  {
    for(int i=0;i<listL;i++)
    {
      if(cmdList[i]->match(Gletter,GValue,GCLine))
      {
        return cmdList[i];
      }
    }
    return NULL;
  }
};




template<int setSize>
class CustomGCmdSets_Static:public CustomGCmdSets{
  protected:
  CustomGCmd *s_cmdList[setSize];
  int capacity;
  public:
  CustomGCmdSets_Static():CustomGCmdSets()
  {
    cmdList=s_cmdList;
    capacity=setSize;
    listL=0;
  }

  virtual int add(CustomGCmd *cmd)
  {
    if(listL>=capacity)return -1;
    cmdList[listL]=cmd;
    listL++;
  }

  void set(CustomGCmd **_cmdlist,int _listL)
  {

  }

};

