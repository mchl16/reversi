from copy import deepcopy as cp
import sys
import random

debug=len(sys.argv)>=2 and sys.argv[1]=='debug'

def gen():
    a,b=1,0
    for i in range(4):
        yield (a,b)
        a,b=-b,a

    a,b=1,1
    for i in range(4):
        yield (a,b)
        a,b=-b,a

class reversi:
    def __init__(self):
        self.color=1
        self.board=bytearray(64)
        self.board[27]=2
        self.board[28]=1
        self.board[35]=1
        self.board[36]=2

    def place_piece(self,field):
        self.board[field]=self.color
        f1,f2=field//8,field%8
        for x,y in gen():
            g1,g2=f1+x,f2+y
            while 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==3-self.color:
                g1,g2=g1+x,g2+y
            
            if not (0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==self.color):
                continue
            g1,g2=f1+x,f2+y
            while 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==3-self.color:
                self.board[8*g1+g2]=self.color
                g1,g2=g1+x,g2+y


    def gen_moves(self):
        a,b=1,0
        for i in range(4):
            yield (a,b)
            a,b=-b,a

        a,b=1,1
        for i in range(4):
            yield (a,b)
            a,b=-b,a

    def check_move(self,field):
        if field<0: 
            return True
        f1,f2=field//8,field%8
        if self.board[field]!=0: return False
        for x,y in gen():
            g1,g2=f1+x,f2+y
            while 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==3-self.color:
                g1,g2=g1+x,g2+y
            
            if 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==self.color:
                if g1!=f1+x or g2!=f2+y: return True

    def move(self,field):
        if debug and not self.check_move(field):
            raise Exception("Illegal move")
        
        if field>=0: self.place_piece(field)
        self.color=3-self.color
            

    def print_board(self):
        for i in range(64):
            print('.BW'[self.board[i]],end='\n' if i&7==7 else '')
        print()

    def change_color(self):
        pass

    def f(x):
        i,j=x//8,x%8
        i=i if i<=3 else 7-i
        j=j if j<=3 else 7-j
        return (100,-20,10,5,-20,-50,-2,-2,10,-2,-1,-1,5,-2,-1,-1)[4*i+j]

    def rank(self): #temporary
        res=0
        for i in range(64):
            if self.board[i]==1: res+=reversi.f(i)
            elif self.board[i]==2: res-=reversi.f(i)
        return res

    def play(self):
        while True:
            self.print_board()
            dirty=False
            for i in range(64):
                if self.check_move(i):
                    dirty=True
                    break
            if not dirty: self.color=3-self.color

            s=list(map(int,input().split()))
            try:
                self.move(8*s[0]+s[1])
            except Exception as e:
                print(e)
            
cnt=0

g=reversi()
print("RDY")

if debug:
    g.play()
else:
    while True:
        xd=input().split()
        cmd,args=xd[0],list(map(int,xd[3::]))

        if cmd=="HEDID":
            g.move(8*args[1]+args[0])
            # g.print_board()
            t=list(filter(lambda x: g.check_move(x),range(64)))
            x=random.choice(t) if len(t)>0 else -1
            print("IDO",-1 if x<0 else x%8,x//8)
            g.move(x)
            # g.print_board()
        elif cmd=="ONEMORE":
            g=reversi()
            print("RDY")
        elif cmd=="BYE":
            exit(0)
        elif cmd=="UGO":
            t=list(filter(lambda x: g.check_move(x),range(64)))
            x=random.choice(t) if len(t)>0 else -1
            print("IDO",-1 if x<0 else x%8,x//8)
            g.move(x)
            # g.print_board()