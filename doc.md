**Input**:$U,F,f,g,S$  
**OutPut**:$h$   
**Algo**:  

---
&emsp;*set $h$'s initial value is $0$*   
&emsp;**DFS**($i$-th elem in $U$,$\lambda$,$p$)   
&emsp;&emsp;**If** i $=$ $|U|$:   
&emsp;&emsp;&emsp;**For** $(x,y)\in S$:   
&emsp;&emsp;&emsp;&emsp;$h((x,y),g(\lambda)(x,y))\gets h((x,y),g(\lambda)(x,y)) + p$   
&emsp;&emsp;&emsp;**EndFor**   
&emsp;&emsp;**Else**:   
&emsp;&emsp;&emsp;$(a,b)\gets$ $i$-th elem in $U$   
&emsp;&emsp;&emsp;**For** $c\in F$:   
&emsp;&emsp;&emsp;&emsp;*set* $\lambda(a,b)=c$   
&emsp;&emsp;&emsp;&emsp;**DFS**($(i+1)$-th elem in $U$, $\lambda$, $p\times f((a,b))(c)$)   
&emsp;&emsp;&emsp;**EndFor**   
&emsp;&emsp;**EndIf**   
&emsp;**EndFunction**