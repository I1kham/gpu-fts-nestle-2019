#ifndef _rheaArray_h_
#define _rheaArray_h_
#include "rheaLinearBuffer.h"

namespace rhea
{
	/*===============================================================================
	 * Array
	 */
	template <typename T> 
	class Array
	{
	public:
		typedef bool (*BubblesortFn) (const T &a, const T &b);
					//ritorna true se devo swappare a con b

	public:
						Array  ()																						{ nElem = nallocati = 0; }
						~Array()																						{ unsetup(); }

						//======================================= memory
		void			setupWithBase (void *startingBlock, u32 sizeOfStartingBlock, Allocator *backingallocator)		{ memBlock.setupWithBase (startingBlock, sizeOfStartingBlock, backingallocator); nallocati=sizeOfStartingBlock/sizeof(T); }
		void			setup (Allocator *backingallocator, u32 prealloc=0)												{ memBlock.setup (backingallocator, prealloc*sizeof(T)); nallocati=prealloc; }
		void			unsetup ()																						{ priv_Destruct (0, nElem); memBlock.unsetup(); nElem = nallocati = 0; }
		void			prealloc (u32 n)																				{ priv_grow (n); }


						//======================================= get / set
		T&				append (const T& t)																				{ T *ret = priv_insert(nElem);  (*ret)=t; return *ret; }
        T&				operator[] (u32 i)																				{ assert(i>=0); return *priv_insert(i); }
        const T&		getElem (u32 i)	const																			{ assert(i<nElem && i>=0);	return *(T*)memBlock._getPointer(i *sizeof(T)); }
        const T&		operator() (u32 i)	const																		{ assert(i<nElem && i>=0);	return *(T*)memBlock._getPointer(i *sizeof(T)); }

						//======================================= copy
		void			copyFrom (const Array<T> &source)
						{
							reset();
							prealloc (source.nElem);
							for (u32 i=0; i<source.nElem; i++)
								(*this)[i] = source(i);
						}


						//======================================= fn
		void			reset()																							{ trunc(0); }
		void			trunc (u32 quantiElementiLascio)																{ assert (quantiElementiLascio<=nElem); priv_Destruct (quantiElementiLascio, nElem-quantiElementiLascio); nElem = quantiElementiLascio; }
		void			remove (u32 i)																					{ removeFromTo(i,1); }
		void			removeFromTo (u32 iFrom, u32 iHowMany)
						{
							assert (nElem>0 && iHowMany>0 && iFrom+iHowMany<=nElem);
							priv_Destruct (iFrom, iHowMany);
							nElem -= iHowMany;
							u32 nElementiDaMuovere = nElem-iFrom;
							if (nElementiDaMuovere>0)
							{
								const u8 *pt = memBlock._getPointer(0);
								memcpy ((void*)&pt[iFrom * sizeof(T)], (void*)&pt[(iFrom+iHowMany) * sizeof(T)], nElementiDaMuovere * sizeof(T));
							}
						}

		void			swap (u32 i1, u32 i2)
						{
							assert (i1<nElem && i2<nElem);
							if (i1==i2)
								return;
							const u8 *p = memBlock._getPointer(0);
							i1 *= sizeof(T);
							i2 *= sizeof(T);
							u8	swap[sizeof(T)];
							memcpy (&swap, &p[i1], sizeof(T));
							memcpy ((void*)&p[i1], (void*)&p[i2], sizeof(T));
							memcpy ((void*)&p[i2], &swap, sizeof(T));
						}

						//prende l'elemento i-esimo e lo "rimuove" semplicemente copiandoci
						//al suo posto l'ultimo elemento
		void			removeAndSwapWithLast (u32 i)
						{
							assert (nElem>0 && i<nElem);
							priv_Destruct (i, 1);
							--nElem;
							if (i < nElem)
							{
								const u8 *p = memBlock._getPointer(0);
								memcpy ((void*)&p[i * sizeof(T)], (void*)&p[nElem * sizeof(T)], sizeof(T));
							}
						}

		u32				simpleSearch (const T &tofind) const
						{
							for (u32 i=0; i<nElem; i++)
								if (*memBlock._getPointer(i *sizeof(T)) == tofind)
									return i;
							return u32MAX;
						}

						// muove tutti gli elementi dell'array a partire a "aPartireDa" (compreso) di "diQuanto" posizioni in avanti
		void			shiftaAvanti (u32 aPartireDa, u32 diQuanto)
						{
							assert (aPartireDa<nElem && diQuanto>0);
							priv_grow (nElem+diQuanto);
							const u8 *pt = memBlock._getPointer(0);
							u32 nElementiDaMuovere = nElem-aPartireDa;
							memmove ((void*)&(pt[(aPartireDa+diQuanto) * sizeof(T)]), (void*)&(pt[aPartireDa * sizeof(T)]), sizeof(T)*nElementiDaMuovere);
							
							priv_Construct (aPartireDa, diQuanto);
							nElem += diQuanto;
						}

						// muove tutti gli elementi dell'array a partire a "aPartireDa" (compreso) di "diQuanto" posizioni indietro
		void			shiftaIndietro (u32 aPartireDa, u32 diQuanto)
						{
							assert (aPartireDa<nElem && aPartireDa>0 && diQuanto>0 && diQuanto<=aPartireDa);
							priv_Destruct (aPartireDa-diQuanto, diQuanto);

							const u8 *p = memBlock._getPointer(0);
							u32 nElementiDaMuovere = nElem-aPartireDa;
							memcpy ((void*)&(p[(aPartireDa-diQuanto) * sizeof(T)]), (void*)&(p[aPartireDa* sizeof(T)]), sizeof(T)*nElementiDaMuovere);
							nElem -= diQuanto;
						}

		void			bubbleSort (BubblesortFn cmpFn)
						{
							u32 n = getNElem();
							if (n < 2)
								return;

							bool bEsci = false;
							while (bEsci == false)
							{
								--n;
								bEsci = true;
								u32 ct = 0;
								T *t1 = (T*)memBlock._getPointer(ct);
								for (u32 i=0; i<n; i++)
								{
									ct+=sizeof(T);
									T *t2 = (T*)memBlock._getPointer(ct);
									if (cmpFn(*t1, *t2))
									{
										bEsci = false;
										swap (i, i+1);
									}
									t1 = t2;
								}
							}
						}

						//===================== query
		u32				getNElem()	const										{ return nElem; }
		u32				getNAllocatedElem() const								{ assert (nallocati*sizeof(T) <= memBlock.getTotalSizeAllocated()); return nallocati; }
		Allocator*		getAllocator() const									{ return memBlock.getAllocator(); }

				

	private:
		void			priv_grow (u32 nuovoNumMaxDiElementi)
						{
							if (nuovoNumMaxDiElementi <= nallocati)
								return;
							bool b= memBlock.growUpTo (nuovoNumMaxDiElementi * sizeof(T));
							assert(b);
							nallocati = nuovoNumMaxDiElementi; 
						}

		void			priv_Construct (u32 iFrom, u32 n)
						{
							u32 ct = iFrom * sizeof(T);
							while (n--)
							{
								::new ((void*)memBlock._getPointer(ct)) T();
								ct += sizeof(T);
							}
						}

		void			priv_Destruct (u32 iFrom, u32 n)
						{
							u32 ct = iFrom * sizeof(T);
							while (n--)
							{
								T *t = (T*)memBlock._getPointer(ct);
								t->~T();
								ct += sizeof(T);
							}
						}

		T*				priv_insert (u32 atPos)
						{
							if (nallocati < 2)
								priv_grow(32);
							while (atPos >= nallocati)
								priv_grow(nallocati + nallocati/2);

#ifdef _DEBUG
							debugArray = (T*)memBlock._getPointer(0);
#endif
							if (atPos >= nElem)
							{
								priv_Construct (nElem, atPos+1-nElem);
								nElem = atPos+1;
							}

							return (T*)memBlock._getPointer((atPos) * sizeof(T));
						}

	private:
		LinearBuffer		memBlock;
		u32					nElem;
		u32					nallocati;

#ifdef _DEBUG
		const T*			debugArray;
#endif
	};
	
} //namespace rhea
#endif	//_rheaArray_h_
