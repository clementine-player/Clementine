Clementine
==========

Clementine; Windows, Linux ve macOS için bir müzik çalar ve kütüphane organizatörüdür..

- Website: http://www.clementine-player.org/
- Github: https://github.com/clementine-player/Clementine
- Buildbot: http://buildbot.clementine-player.org/grid
- Son Geliştirici Çıktıları: http://builds.clementine-player.org/

Hata/İstek Belirtmek
--------------------
### Yeni bir özellik için istekte bulunmak

Lütfen:
 * İstediğiniz özelliğin daha önce eklenip eklenmediğini kontrol edin. (Sürüm notlarından)
 * Daha önce birisinin bu isteği oluşturmadığından emin olun.
 * Eğer bu istek daha önce yapılmışsa tekrar yorumlamanız işleri hızlandırmaz. Onun yerine, bu isteğe abone olabilirsiniz. Abone olduğunuzda bu başlık altındaki gelişmelerden haberdar olursunuz.

### Hata raporlamak

Lütfen:
 
 * Son geliştirici çıktısınından (http://builds.clementine-player.org/) hatanın geçerliliğini kontrol edin. (**Dikkat**, Bu çıktılar henüz kararlı çıktılar değildir, bu yüzden doğru çalışmayabilir ve kullanıcı ayarlarınıza zarar verebilir). Eğer her şey doğru çalışıyorsa lütfen hata raporu altında bu hatanın giderildiğini açıklayın.
 * Kopyalardan kaçınmak için, birinin daha önce bu hatayı belirtmediğinden emin olun.
 * Eğer hata daha önce belirtilmişse kendi, hatanızın ayrıntılarını yorum olarak veya sizinle aynı hatayla karşılaşanları doğrulayın.
 * Eğer yukarıdaki iki durum geçerli değilse yeni bir hata raporu açın ve detaylarınızı (İşletim Sistemi, Clementine sürümü, hatanın nasıl meydana geldiğini) mutlaka belirtin.
 * Hata kodları ve günlük çıktıları için lütfen http://pastebin.com/ kullanın.
 
Eğer cevap alamazsanız bu demektir ki, oluşturduğunuz istek/hata raporuna vakit ayıramıyoruz. Yani hatayı yeniden işleyemiyoruz veya bunun için zamanımız yok :o.

Kaynaktan Derleme
-----------------

Kodu alın (eğer hala almadıysanız):

    git clone https://github.com/clementine-player/Clementine.git && cd Clementine

Derleyin ve kurun:

    cd bin
    cmake ..
    make -j8
    sudo make install

Daha fazla bilgi ve bağımlılık listesi için Wikiyi ziyaret edebilirsiniz:
https://github.com/clementine-player/Clementine/wiki/Compiling-from-Source
