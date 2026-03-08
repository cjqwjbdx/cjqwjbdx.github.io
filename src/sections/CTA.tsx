import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';

gsap.registerPlugin(ScrollTrigger);

const CTA = () => {
  const sectionRef = useRef<HTMLElement>(null);
  const bgRef = useRef<HTMLDivElement>(null);
  const contentRef = useRef<HTMLDivElement>(null);
  const buttonsRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const section = sectionRef.current;
    const bg = bgRef.current;
    const content = contentRef.current;
    const buttons = buttonsRef.current;

    if (!section || !bg || !content || !buttons) return;

    const ctx = gsap.context(() => {
      // Background image fade in
      gsap.fromTo(
        bg,
        { opacity: 0 },
        {
          opacity: 0.3,
          duration: 1,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: section,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Title word reveal
      const titleWords = content.querySelectorAll('.title-word');
      titleWords.forEach((word, index) => {
        gsap.fromTo(
          word,
          { opacity: 0, y: 50 },
          {
            opacity: 1,
            y: 0,
            duration: 0.6,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: content,
              start: 'top 75%',
              toggleActions: 'play none none reverse',
            },
            delay: index * 0.1,
          }
        );
      });

      // Subtitle
      gsap.fromTo(
        content.querySelector('.subtitle'),
        { opacity: 0, y: 20 },
        {
          opacity: 1,
          y: 0,
          duration: 0.5,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: content,
            start: 'top 70%',
            toggleActions: 'play none none reverse',
          },
          delay: 0.4,
        }
      );

      // Buttons
      const buttonElements = buttons.querySelectorAll('button');
      buttonElements.forEach((button, index) => {
        gsap.fromTo(
          button,
          { opacity: 0, scale: 0 },
          {
            opacity: 1,
            scale: 1,
            duration: 0.5,
            ease: 'back.out(1.7)',
            scrollTrigger: {
              trigger: buttons,
              start: 'top 80%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.6 + index * 0.1,
          }
        );
      });

      // Parallax effects
      gsap.to(bg, {
        y: -50,
        ease: 'none',
        scrollTrigger: {
          trigger: section,
          start: 'top bottom',
          end: 'bottom top',
          scrub: 0.5,
        },
      });

      gsap.to(content, {
        y: 30,
        ease: 'none',
        scrollTrigger: {
          trigger: section,
          start: 'top bottom',
          end: 'bottom top',
          scrub: 0.5,
        },
      });
    }, section);

    return () => ctx.revert();
  }, []);

  return (
    <section
      id="cta"
      ref={sectionRef}
      className="relative min-h-[80vh] flex items-center justify-center bg-black overflow-hidden"
    >
      {/* Background Product Image */}
      <div
        ref={bgRef}
        className="absolute inset-0 opacity-0"
      >
        <img
          src="/images/product-white.png"
          alt=""
          className="w-full h-full object-cover opacity-30"
        />
        <div className="absolute inset-0 bg-gradient-to-t from-black via-black/80 to-black/60" />
      </div>

      {/* Content */}
      <div className="relative z-10 max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 text-center">
        <div ref={contentRef}>
          <h2 className="heading-section text-white mb-6">
            <span className="title-word inline-block">准备好</span>{' '}
            <span className="title-word inline-block">升级</span>{' '}
            <span className="title-word inline-block">你的</span>{' '}
            <span className="title-word inline-block">日历了吗？</span>
          </h2>
          <p className="subtitle text-xl text-gray-400 mb-10 max-w-2xl mx-auto">
            立即购买，享受科技带来的便捷
          </p>
        </div>

        <div ref={buttonsRef} className="flex flex-col sm:flex-row gap-4 justify-center">
          <a
            href="https://www.goofish.com/personal?spm=a21ybx.item.itemHeader.1.c03b3da6zaL1ag&userId=2219084601045"
            target="_blank"
            rel="noopener noreferrer"
            className="btn-primary text-lg px-8 py-4"
            style={{
              boxShadow: '0 0 0 0 rgba(0, 113, 227, 0.4)',
              transition: 'all 0.3s ease, box-shadow 0.3s ease',
            }}
            onMouseEnter={(e) => {
              e.currentTarget.style.boxShadow = '0 0 30px 10px rgba(0, 113, 227, 0.3)';
            }}
            onMouseLeave={(e) => {
              e.currentTarget.style.boxShadow = '0 0 0 0 rgba(0, 113, 227, 0.4)';
            }}
          >
            立即购买
          </a>
          <button
            onClick={() => document.querySelector('#features')?.scrollIntoView({ behavior: 'smooth' })}
            className="px-8 py-4 rounded-full text-white font-medium text-lg border border-white/30 hover:bg-white hover:text-black transition-all duration-300"
          >
            了解更多
          </button>
        </div>

        {/* Trust Badges */}
        <div className="mt-16 flex flex-wrap justify-center gap-8 text-gray-500 text-sm">
          <div className="flex items-center gap-2">
            <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
            </svg>
            <span>非人为因素收货当日无理由退货</span>
          </div>
          <div className="flex items-center gap-2">
            <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 12l2 2 4-4m5.618-4.016A11.955 11.955 0 0112 2.944a11.955 11.955 0 01-8.618 3.04A12.02 12.02 0 003 9c0 5.591 3.824 10.29 9 11.622 5.176-1.332 9-6.03 9-11.622 0-1.042-.133-2.052-.382-3.016z" />
            </svg>
            <span>好评，推荐返现4元</span>
          </div>
          <div className="flex items-center gap-2">
            <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M18.364 5.636l-3.536 3.536m0 5.656l3.536 3.536M9.172 9.172L5.636 5.636m3.536 9.192l-3.536 3.536M21 12a9 9 0 11-18 0 9 9 0 0118 0zm-5 0a4 4 0 11-8 0 4 4 0 018 0z" />
            </svg>
            <span>技术支持</span>
          </div>
        </div>
      </div>
    </section>
  );
};

export default CTA;
