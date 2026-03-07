
= ChainlitのUIを@<ruby>{癖, へき}に染める

== はじめに
LLMが一般社会に急速に浸透し、我々の生活やワークスタイルは大きく変化しました。日々の献立から専門的な分野の調査分析まで、普段使う言葉で対話しながら解決できるLLMは、もはや日常になくてはならないものになっています。それはまるで、先人たちがSF作品で描いたような世界のようです。
ゲームに絞っても、人工知能との関わりを描いた作品は数多く思い当たります。「ナビ」と呼ばれる人工知能を使ってインターネットウイルスを駆逐したり、2つのポータルと人間の知恵を駆使して暴走した人工知能と対峙したり。もしかしたらLLMに対し、単なるツールを超えた人格としての親近感を覚える方もいるのではないでしょうか。少なくとも私はそうです。
このような作品では、世界観に合わせて近未来的なUIが導入されることが多い傾向にあります。その鮮烈なネオンカラーと非自然的なポリゴンデザインの格好良さは色褪せません。現実世界に存在するLLMのUIデザインはシンプルなものが多いですが、その先入観を排してSF風に寄せてみたら、日々の業務がちょっと楽しくなるのでは？という思いつきから、ChainlitのUI変更に挑戦することにしました。

== この記事の取扱範囲と資料について
=== 取扱範囲・取り扱わない範囲
この記事ではChainlitに対するCSS, JSの適用による要素の変更を取り扱います。
想定レベルは「普段pythonやその他の言語を利用して分析・開発を行っているひと」
下記の項目は取り扱いません。
- CSS, JSの専門的な解説
- tailwind(ChainlitはShadcn/tailwindベースですが今回は利用しません)

=== 参考資料について
題材の性質上、この記事にはカラーで見た方が箇所がたびたび登場します。ただ、書籍は白黒印刷のため、下記のサイトに参考資料を掲載しました。
記事内でも必要な箇所にサイトに遷移するQRコードを掲載しますので、スマホ片手にご覧ください。
link
QR Code

== ChainlitのUI変更の仕様を知る
ChainlitはLLMとの会話アプリを簡単に実装することを主眼に作られています。フロントエンド開発の知識が乏しくてもアプリを構築できるよう、標準で使いやすいチャットUIが用意されています。これらはReact + Tailwind + shadcn/uiの構成でChainlit内部に実装されており、通常のプロジェクト作成時にはフロントエンドのソースコードはプロジェクトディレクトリに生成されません。
ただし、簡単なデザイン変更はサポートされています。具体的には、config.tomlに装飾用のファイル（css, js, faviconなど）のpathを書き込むことで、標準UIのスタイルを上書きできます。
整理すると、HTMLは編集不可、CSSとJSは適用可能という構図です。そのため、@<b>{標準UIをCSSで装飾しつつ、必要であればjsでHTMLの要素を編集する}という作戦で実装することにします。

== 目標
実装にあたり、Figmaでトンマナ案とデザイン案を作成しました。カラー版はリンクをご覧ください。
あくまでも案として、実装による多少の差異は許容することにします。

image, QR

また、今回の実装にあたり、円をモチーフにしたロゴを作成しました。最終的にはこのロゴを回転させます。

image

== 変更していこう

=== ロゴを変更する
手始めに、ロゴを変えていきましょう。このロゴはfaviconにも使われます。まず、.chainlitディレクトリと同じ階層にpublicディレクトリを作り、faviconとロゴに使うファイルを配置します。

//emlist[config.yamlとpublic]{
その他のディレクトリは省略
.chainlit
├── translations/ …省略
└── config.yaml

public/
└── logo.svg
//}

その後、config.yamlに移動し、logo_file_urlのコメントアウトを解除します。そして配置したロゴファイルのpathを入力すれば完了です。簡単！ついでに、AIの返答メッセージの横に表示されるアイコンも同じファイルにしておきましょう。

//emlist[config.yamlの編集][python]{
# Load assistant logo directly from URL.
logo_file_url = "/public/logo.svg"

# Load assistant avatar image directly from URL.
default_avatar_file_url = "/public/logo.svg"
//}

補足として、config.yamlには他にも設定項目があります。この記事では特に扱いませんが、これらを使えば標準UIのデザインをさらにカスタマイズできるでしょう。

//emlist[その他の設定項目例][python]{
# Custom login page image, relative to public directory or external URL
login_page_image = "/public/custom-background.jpg"

# Custom login page image filter (Tailwind internal filters, no dark/light variants)
login_page_image_filter = "brightness-50 grayscale"
login_page_image_dark_filter = "contrast-200 blur-sm"

# Specify a custom meta image url.
custom_meta_image_url = "https://chainlit-cloud.s3.eu-west-3.amazonaws.com/logo/chainlit_banner.png"
//}

=== 見出しやボタンの表記を変える
.chainlitの配下にtranslationsというディレクトリと多数のjsonが自動生成されます。これらは、ChainlitのUIで使用されるラベルやボタン、メニューなどのテキストの翻訳辞書です。これらの翻訳ファイルを編集することで、UIの表示文言を任意の言葉に変更することができます。デフォルトでは「ログイン」「パスワード」など、日本語を使った表記にしてくれているのですが、今回は見た目重視で英単語で表記されるように変更します。

//emlist[ja.jsonの設定][json]{
    "auth": {
        "login": {
            "title": "SYSTEM LOGIN",
            "form": {
                "email": {
                    "label": "USER ID",
                    "required": "USER ID IS REQUIRED / USER ID が必要です",
                    "placeholder": "USER ID"
                },
                "password": {
                    "label": "PASSWORD",
                    "required": "PASSWORD IS REQUIRED / PASSWORD が必要です",
                    "placeholder": "Password"
                },
                "actions": {
                    "signin": "LOGIN"
                },
                "alternativeText": {
                    "or": "\u307e\u305f\u306f"
                }
            },
//}


ここまでで、ロゴと表記を変更できました。今の見た目はこのようになっています。

//image[seahawk-default-login][変更前][scale=0.35]
//image[seahawk-customized-login][変更後][scale=0.35]


=== 静的要素を変更する

==== 初期設定とcustom_themeについて
ここからは、CSSを編集して静的要素を変更していきます。そのために、public内部にcssファイル, theme.jsonというファイルを作成し、config.yamlを編集します。ついでに、後ほど使うjsファイルも作成し、設定もしておきましょう。
theme.json は、ChainlitのUIで使用されるカラーパレットやデザイントークンを定義するためのファイルです。
このファイルを利用することで、アプリケーション全体の配色やUIテーマを統一することができます。
custom_css や custom_js は .chainlit/config.tomlにコメント付きの設定例が記載されていますが、custom_themeの設定はテンプレートに含まれていない場合があります。そのため、テーマを利用する場合は config.tomlにcustom_themeの設定を手動で追加し、theme.jsonを自分で作成する必要があります。

//emlist[cssファイル作成]{
その他のディレクトリは省略
.chainlit
├── translations/ …省略
└── config.yaml

public/
├── logo.svg
├── theme.json <- 新規作成
├── effects.js <- 新規作成
└── stylesheet.css <- 新規作成
//}

//emlist[config.yamlの編集][python]{
custom_theme = "/public/theme.json"

# Specify a CSS file that can be used to customize the user interface.
# The CSS file can be served from the public directory or via an external link.
custom_css = "/public/stylesheet.css"

# Specify a JavaScript file that can be used to customize the user interface.
# The JavaScript file can be served from the public directory.
custom_js = "/public/effects.js"
//}

ここに、あらかじめ決めておいたトンマナを記載しておきます。詳細はサンプルコードをご覧ください。
//emlist[theme.jsonのサンプル][json]{
{
  "custom_fonts": [],
  "variables": {
    "light": {
      "--background": "0 0% 100%",
      "--foreground": "0 0% 5%",
      ...
    },
    "dark": {
      "--background": "222 47% 6%",
      "--foreground": "222 40% 96%",
      ...
    }
  }
}
//}

==== ログイン画面の変更
基本的に、これからは@<b>{selectorを特定し、cssやjsで書き換える}という作業を続けていきます。
目標のデザインを実現する上で、大きな改造が必要なのは下記の2つです。


* 標準UIの背景のロゴを消し、入力欄を中央に配置する
* ロゴを入力欄の上部に表示する

===== 標準UIのロゴを消して、入力欄を中央に配置する
まず、ログイン画面の判定のために、下記のセレクタを利用します。
//emlist[]{
body:has(input[type="password"]):has(button[type="submit"])
//}

その上で、標準UIの背景画像は下記のセレクタで指定できるので、まるっと消します。
//emlist[]{
body:has(input[type="password"]):has(button[type="submit"])
  img.absolute.inset-0.h-full.w-full.object-cover {
  display: none !important;
}
//}

続いて、フォームを中央に配置する処理です。その上でフォームを含むrootを全面に広げ、中央配置します。その後、フォームを持つ要素をセンター寄せします。
//emlist[]{
body:has(input[type="password"]):has(button[type="submit"]) #root,
body:has(input[type="password"]):has(button[type="submit"]) main{
  ...
  width: 100%;
  place-items: center;
  ...
}

body:has(input[type="password"]):has(button[type="submit"])
  main:has(form) {
  ...
  justify-items: center !important;
  align-items: center !important;
  ...
}

body:has(input[type="password"]):has(button[type="submit"])
  main:has(form) > :has(form) {
  ...
  place-items: center !important;
  ...
}
//}

===== ロゴを入力欄の上部に表示する
ロゴを画面の中心かつ入力欄の上に配置するため、logo.svgを使った要素を探して、センター寄せします。

//emlist[]{
body:has(input[type="password"]):has(button[type="submit"])
  :is(img.logo, img[src="/public/logo.svg"], img[src="/logo"]) {
  display: block !important;
  width: 180px;
  height: auto;
  margin: 0 auto 24px auto !important;
}
//}

===== ボタン
あとはフォームの要素や見出し、ボタンを装飾します。下記は入力ボックスの例です。ログイン画面全体のinputを指定しており、同じ要領でbuttonも操作をしています。

//emlist[]{
body:has(input[type="password"]):has(button[type="submit"]) input {
  width: 100%;
  height: 42px;
  background: hsl(var(--background));
  color: hsl(var(--foreground));
  border: 1px solid hsl(var(--border));
  transition: border-color 0.2s ease, box-shadow 0.2s ease, filter 0.2s ease;
  border-radius: 0 !important;
  padding: 0 14px;
  outline: none;
  box-shadow:
    0 0 10px hsl(var(--primary) / 0.12);
}
//}

これらの処理により、目標の見た目に近づきました。
//image[seahawk-customized-login-2][良い感じになってきた][scale=0.5]

==== トーク画面の変更
ログイン画面に比べて、トーク画面の変更は比較的シンプルです。トーク画面の判定には下記のセレクタを利用します。

//emlist[]{
body:not(:has(input[type="password"]):has(button[type="submit"]))
//}

トーク画面には入力ボックス・ユーザーバブル・アシスタントバブル（LLM側の返答）があり、それぞれのカスタマイザは下記の通りです。アシスタントバブルはユーザーバブルに該当するclassの除外により分別しています。

//emlist[]{
/* 入力ボックスのselector */
body:not(:has(input[type="password"]):has(button[type="submit"])) #message-composer

/* ユーザーバブルのselector */
body:not(:has(input[type="password"]):has(button[type="submit"]))
  div.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0

/* アシスタントバブルのselector */
body:not(:has(input[type="password"]):has(button[type="submit"]))
  .message-content:not(.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0 .message-content)
//}

===== ユーザーバブルの装飾
標準UIでは、ユーザーバブルには角丸四角の装飾が施されています。
//image[seahawk-default-bubble][標準のバブルデザイン][scale=0.5]

実装ではこの要素を非表示にした上で、枠の装飾を適用しています。
//emlist[]{
body:not(:has(input[type="password"]):has(button[type="submit"]))
  div.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0 .message-content {
  background: transparent !important;
  border: 0 !important;
  ...
}
//}

===== アシスタントバブルの装飾
ユーザーバブルとは異なり、左にアイコン、その横に返答メッセージが表示されます。囲み枠はありません。
枠自体の適用はユーザーバブルと同様ですが、アイコンを含む分の位置調整が必要になります。

//emlist[]{
span.relative.flex.shrink-0.overflow-hidden.rounded-full[data-state]:has(> img[alt="Avatar for Assistant"]) img[alt="Avatar for Assistant"] {
  ...
  left: -30px;
  ...
}
//}

以上の編集を終えて、静的デザインは完成です。
//image[seahawk-customized-talk-1][トーク初期画面][scale=0.5]
//image[seahawk-customized-talk-2][トーク中の画面][scale=0.5]

=== 動的要素を変更する
ここまでで、静的な適用はできてきました。ここからは、js要素の活用の確認と、近未来感の醸成のために、動きをつけていきましょう。
custom_jsの設定は前項を参照ください。

==== バブルに演出をつける
メッセージを送信したあとや、LLM側の返答が表示されるときにチカチカと光るような装飾をつけます。

(image)

そのために、jsで対象要素の監視とclass付与を行います。@<table>{class-role}にclassと役割の対応を示します。対象は前項で解説したselectorで指定します。
//table[class-role][classと役割について]{
target 'cl-anim-target' 装飾適用対象の要素につける
active 'cl-anim-enter-active' 装飾を実際に表示している要素につける
done 'cl-anim-done' 装飾を表示し終わった要素につける
//}

詳細はソースコードをご確認いただきたいと思いますが、classの付与はanimateElement関数で制御しています。

//emlist[handleMutatins内]{
  function animateElement(element) {
    if (!isElement(element)) return;
    if (!hasRenderableContent(element)) return;
    if (isAnimated(element) || isAnimating(element)) return;

    element.dataset[DATA_ATTR.animated] = 'pending';
    element.classList.add(CLASS_NAMES.target);
    element.classList.remove(CLASS_NAMES.done);

    debugLog('animation start', element);

    requestAnimationFrame(() => {
      void element.offsetWidth;
      requestAnimationFrame(() => {
        element.classList.add(CLASS_NAMES.active);
      });
    });

    window.setTimeout(() => {
      markAsDone(element);
    }, ENTER_DURATION_MS);
  }
//}

あとは、cssでblinkの挙動を書けば完成です。

//emlist[blinkするCSS]{
@keyframes cl-border-blink-twice {
  0% {
    background: var(--chat-frame-border);
  }
  15% {
    background: var(--chat-enter-flash-border);
  }
  30% {
    background: var(--chat-frame-border);
  }
  70% {
    background: var(--chat-enter-flash-border);
  }
  100% {
    background: var(--chat-frame-border);
  }
}

body:not(:has(input[type="password"]):has(button[type="submit"]))
  div.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0.cl-anim-target.cl-anim-enter-active::before,
body:not(:has(input[type="password"]):has(button[type="submit"]))
  .message-content:not(.px-5.py-2\.5.relative.bg-accent.rounded-3xl.max-w-\[70\%\].flex-grow-0 .message-content).cl-anim-target.cl-anim-enter-active::before {
  ...
  animation: cl-border-blink-twice 320ms linear 1;
}
//}

==== ロゴを無駄に回す
最後に、円形モチーフのロゴを作りましたが、せっかくなのでゆっくりと回るようにしたいと思います。ロゴは半径と色、太さの異なる複数の円を重ねて作っています。このそれぞれの縁を異なるスピードで回転させます。svgでそれぞれのパーツは書き出してあるので、2ステップに分けて実装していきます。

===== publicにロゴを置く


//emlist[]{
public/
└── logos/
    ├── ring-01.svg
    ├── ring-02.svg
    ├── ring-03.svg
    └── ...
//}



===== ロゴを複数枚のsvgの重ね合わせに置き換える
Chainlitの標準では、ロゴは1枚のみ登録可能です。一方、今回は複数のsvgを重ね合わせるので、jsで要素を上書きする必要があります。

//emlist[既存のロゴを重ね合わせたロゴに置き換える][javascript]{
/* 重ね合わせロゴのコンテナを作る */
function createLogoContainer(className) {
    const container = document.createElement('div');
    container.className = className;
    container.setAttribute('aria-hidden', 'true');

    LOGO_LAYERS.forEach((src, i) => {
        const layer = document.createElement('img');
        layer.src = src;
        layer.alt = '';
        layer.className = `cl-logo-layer cl-logo-layer-${i + 1}`;
        container.appendChild(layer);
    });

    return container;
}

/* 置き換える */
function replaceLoginLogo() {
    const form = document.querySelector('form:has(input[type="password"])');
    ...
    form.prepend(createLogoContainer('cl-layered-logo'));
    form.dataset.clLogoInjected = 'true';
    ...
}
//}

===== ロゴを回す
上の処理で、それぞれのレイヤーに cl-logo-layer-${i + 1} と連番のclassを付与しました。
このclassを使って回転を定義すればOKです。常に回し続ければ良いので、擬似要素での分岐もなく楽ですね。

//emlist[既存のロゴを重ね合わせたロゴに置き換える][css]{
:is(.cl-layered-logo) .cl-logo-layer-1 { animation-duration: 16s; }
:is(.cl-layered-logo) .cl-logo-layer-2 { animation-duration: 25s; }
:is(.cl-layered-logo) .cl-logo-layer-3 { animation-duration: 18s; }
:is(.cl-layered-logo) .cl-logo-layer-4 { animation-duration: 26s; }
:is(.cl-layered-logo) .cl-logo-layer-5 { animation-duration: 21s; }
:is(.cl-layered-logo) .cl-logo-layer-6 { animation-duration: 17s; }
:is(.cl-layered-logo) .cl-logo-layer-7 { animation-duration: 24s; }
:is(.cl-layered-logo) .cl-logo-layer-8 { animation-duration: 19s; }
//}

これで、ロゴの回転が実装できました。

=== 完成！
以上で出来上がったものがこちらのrepositoryに格納されています。
link

また、ムービーを以下のサイトにあげているのでぜひご覧ください。
link
QRの画像

== 終わりに
selectorを特定してcssとjsで操作するという地味な作業が多い記事でしたが、Reactやtailwindなどの著名なjsのフレームワークを使わなくてもUIを変更できました。もちろん、もっと凝ったデザインを志向するのであればそちらを使った方がいいと思います。ただ、標準UIの構成自体は複雑ではなく、pythonを使い慣れた分析寄りのスキルセットを持った方々には十分かつ丁寧な仕様と言えるでしょう。もっと言えば、標準UIも洗練されていて、theme.jsonでトンマナを変えるだけでも良いのではと思ったぐらいです。
UIデザインは、プロダクトの利便性の根幹の一つであると同時に、製作者がプロダクトに込めた思いを表現する場でもあります。もし業務やプライベートでChainlitを使うときは、ちょっと凝ったデザインにして、その思いをこっそり色や形に忍ばせてみてはいかがでしょうか。

== 参考資料
https://docs.chainlit.io/customisation/overview
https://developer.mozilla.org/ja/docs/Web/CSS
