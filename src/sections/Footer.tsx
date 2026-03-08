import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';

gsap.registerPlugin(ScrollTrigger);

const Footer = () => {
  const footerRef = useRef<HTMLElement>(null);
  const contentRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const footer = footerRef.current;
    const content = contentRef.current;

    if (!footer || !content) return;

    const ctx = gsap.context(() => {
      gsap.fromTo(
        content,
        { opacity: 0 },
        {
          opacity: 1,
          duration: 0.6,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: footer,
            start: 'top 90%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      const links = content.querySelectorAll('a');
      links.forEach((link, index) => {
        gsap.fromTo(
          link,
          { opacity: 0, y: 10 },
          {
            opacity: 1,
            y: 0,
            duration: 0.3,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: footer,
              start: 'top 90%',
              toggleActions: 'play none none reverse',
            },
            delay: index * 0.05,
          }
        );
      });
    }, footer);

    return () => ctx.revert();
  }, []);

  const footerLinks = [
    { name: '隐私政策', href: '#' },
    { name: '使用条款', href: '#' },
    { name: '销售政策', href: '#' },
    { name: '法律信息', href: '#' },
  ];

  return (
    <footer
      ref={footerRef}
      className="py-12"
      style={{ backgroundColor: 'var(--apple-light-gray)' }}
    >
      <div ref={contentRef} className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        {/* Logo */}
        <div className="flex justify-center mb-8">
          <div className="flex items-center gap-2">
            <img
              src="/images/logo.jpeg"
              alt="WuJin Studio"
              className="h-8 w-auto"
            />
            <span className="text-lg font-semibold text-[var(--apple-dark)]">
              WuJin Studio
            </span>
          </div>
        </div>

        {/* Links */}
        <div className="flex flex-wrap justify-center gap-6 mb-8">
          {footerLinks.map((link) => (
            <a
              key={link.name}
              href={link.href}
              className="text-sm text-[var(--apple-gray)] hover:text-[var(--apple-dark)] transition-colors duration-200 relative group"
            >
              {link.name}
              <span className="absolute -bottom-1 left-0 w-0 h-px bg-[var(--apple-dark)] transition-all duration-200 group-hover:w-full" />
            </a>
          ))}
        </div>

        {/* Divider */}
        <div className="border-t border-gray-200 my-8" />

        {/* Copyright */}
        <div className="text-center">
          <p className="text-sm text-[var(--apple-gray)]">
            &copy; {new Date().getFullYear()} WuJin Studio. 保留所有权利。
          </p>
          <p className="text-xs text-[var(--apple-gray)] mt-2">
            匠心科技，融入生活
          </p>
        </div>
      </div>
    </footer>
  );
};

export default Footer;
