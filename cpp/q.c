
#define m_size(p)  ((p) [0].size)	/* For malloc */
#define m_next(p)  ((p) [1].next)	/* For malloc and alloca */
#define m_deep(p)  ((p) [0].depth)	/* For alloca */

	       m_size(p1) += m_size(m_next(p1));
	       m_next(p1) = m_next(m_next(p1));
	       noise("JOIN 2", p1);

     hello/?? 
??=warning oooer
#define LOCK_NB		4	/* or'd with one of the above to prevent
				   blocking */
#define LOCK_UN		8	/* remove lock */


#define comba(x,y) x/**/y
#define combb(x,y) x ## y


   comba(un,signed);
   combb(un,signed);
.
